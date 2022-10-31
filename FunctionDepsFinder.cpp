#include "FunctionDepsFinder.hh"
#include "PrettyPrint.hh"
#include "clang/Analysis/CallGraph.h"
#include "clang/Sema/IdentifierResolver.h"

/** Build CallGraph from AST.
 *
 * The CallGraph is a datastructure in which nodes are functions and edges
 * represents function call points. For example:
 *
 * void f();
 * void g() { f(); f(); }
 *
 * Resuluts in the following CallGraph:
 *
 * (f) -> (g)
 *     -> (g)
 *
 * There are two edges to `g` because there are two callpoints to it.
 * Hence, the resulting graph is not `simple` in Graph Theory nomenclature. But
 * for the analysis we are doing it is suffice to be, so perhaps some extra
 * performance can be archived if we could remove duplicated edges.
 *
 */
CallGraph *Build_CallGraph_From_AST(ASTUnit *ast)
{
  CallGraph *cg = new CallGraph();

  /* Iterate trough all nodes in toplevel.  */
  clang::ASTUnit::top_level_iterator it;
  for (it = ast->top_level_begin(); it != ast->top_level_end(); ++it) {
    Decl *decl = *it;

    /* Add decl node to the callgraph.  */
    cg->addToCallGraph(decl);
  }

  return cg;
}

void FunctionDependencyFinder::Find_Functions_Required
            (CallGraph *cg, std::vector<std::string> const& funcnames)
{
  assert(funcnames.size() > 0);

  for (const std::string &funcname : funcnames) {
    /* Find the node which name matches our funcname.  */
    clang::CallGraph::iterator it;
    for (it = cg->begin(); it != cg->end(); ++it) {
      CallGraphNode *node = (*it).second.get();

      /* Get Declaration as a NamedDecl to query its name.  */
      NamedDecl *ndecl = dynamic_cast<NamedDecl *>(node->getDecl());

      if (ndecl && ndecl->getNameAsString() == funcname) {
        // Now iterate through all nodes reachable from begin_node and mark all
        // of them for output.
        Mark_Required_Functions(node);
      }
    }
  }
}

void FunctionDependencyFinder::Mark_Required_Functions(CallGraphNode *node)
{
  if (!node)
    return;

  /* If the function is already in the FunctionDependency set, then we can
     quickly return because this node was already analyzed.  */
  if (Is_Decl_Marked(node->getDecl()))
    return;

  FunctionDecl *func_decl = node->getDecl()->getAsFunction();

  /* In case this function has a body, we mark its body as well.  */
  Add_Decl_And_Prevs(func_decl->hasBody()? func_decl->getDefinition(): func_decl);

  /* Iterate through all callees and repeat the process in a DFS fashion.  */
  for (CallGraphNode *callee: *node) {
    Mark_Required_Functions(callee);
  }
}

void FunctionDependencyFinder::Compute_Closure(void)
{

  size_t n = 0;
  Decl *dependencies[Dependencies.size()];

  /* Copy the Dependencies set into a temporary array so we don't shoot ourselves
     on the foot.  */
  for (Decl *decl: Dependencies) {
    /* If decl is a function, then add to analysis queue.  */
    if (decl->getAsFunction()) {
      dependencies[n++] = decl;
    }
  }

  /* Iterate through all function definitions and trigger the
     RecordDecl dependency tracker.  */
  for (size_t i = 0; i < n; i++) {
    FunctionDecl *decl = dependencies[i]->getAsFunction();
    Mark_Types_In_Function_Body(decl->getBody());
  }

  /* Handle the corner case where an global array was declared as

    enum {
      const1 = 1,
    }

    int var[cost1];

    in which clang will unfortunatelly expands const1 to 1 and lose the
    EnumConstantDecl in this case, forcing us to reparse this declaration.  */
  for (Decl *decl: Dependencies) {
    VarDecl *vardecl = dynamic_cast<VarDecl *>(decl);
    if (vardecl) {
      Handle_Array_Size(vardecl);
    }
  }
}

bool FunctionDependencyFinder::Add_Type_And_Depends(const Type *type)
{
  if (!type)
    return false;

  bool inserted = false;

  /* Check type to the correct type and handle it accordingly.  */
  if (TagType::classof(type)) {
    /* Handle case where type is a struct, union, or enum.  */

    const TagType *t = type->getAs<const TagType>();
    return Handle_TypeDecl(t->getAsTagDecl());
  }

  if (type->isTypedefNameType()) {
    /* Handle case where type is a typedef.  */

    const TypedefType *t = type->getAs<const TypedefType>();
    inserted = Handle_TypeDecl(t->getDecl());

    /* Typedefs can be typedef'ed in a chain, for example:
         typedef int __int32_t;
         typedef __int32_t int32_t;

         int f(int32_t);

         In this case we must also include the first typedef.

         Also only do this if something was inserted, else there is
         no point in continuing this analysis because the decl
         was already marked for output.
    */

    if (inserted)
      Add_Type_And_Depends(t->desugar().getTypePtr());

    return inserted;
  }

  if (type->isFunctionProtoType()) {
    /* Handle function prototypes.  Can appear in typedefs or struct keys
       in ways like this:

        typedef int (*GEN_SESSION_CB)(int *, unsigned char *, unsigned int *);

       */
    const FunctionProtoType *prototype = type->getAs<const FunctionProtoType>();
    int n = prototype->getNumParams();

    /* Add function return type.  */
    inserted |= Add_Type_And_Depends(prototype->getReturnType().getTypePtr());

    /* Add function parameters.  */
    for (int i = 0; i < n; i++) {
      inserted |= Add_Type_And_Depends(prototype->getParamType(i).getTypePtr());
    }

    return inserted;
  }

  if (type->isElaboratedTypeSpecifier()) {
    /* For some reason clang add some ElaboratedType types, which is not clear
       of their purpose.  But handle them as well.  */

    const ElaboratedType *t = type->getAs<const ElaboratedType>();

    return Add_Type_And_Depends(t->desugar().getTypePtr());
  }
  if (type->isPointerType() || type->isArrayType()) {
    /* Handle case where type is a pointer or array.  */

    return Add_Type_And_Depends(type->getPointeeOrArrayElementType());
  }

  return false;
}

bool FunctionDependencyFinder::Handle_TypeDecl(TypeDecl *decl)
{
  bool inserted = false;

  /* If decl was already inserted then quickly return.  */
  if (Is_Decl_Marked(decl))
    return false;

  /* Add current decl.  */
  inserted = Add_Decl_And_Prevs(decl);

  if (RecordDecl *rdecl = dynamic_cast<RecordDecl *>(decl)) {
    /* RecordDecl may have nested records, for example:

      struct A {
        int a;
      };

      struct B {
        struct A a;
        int b;
      };

      in this case we have to recursively analyze the nested types as well.  */
    clang::RecordDecl::field_iterator it;
    for (it = rdecl->field_begin(); it != rdecl->field_end(); ++it) {
      ValueDecl *valuedecl = *it;
      Handle_Array_Size(valuedecl);

      Add_Type_And_Depends(valuedecl->getType().getTypePtr());
    }
  }

  return inserted;
}

void FunctionDependencyFinder::Mark_Types_In_Function_Body(Stmt *stmt)
{
  if (!stmt)
    return;

  /* For some akward reason we can't use dynamic_cast<>() on Stmt.  So
     we poke in this classof which seems to work.  */
  const Type *type = nullptr;

  if (DeclStmt::classof(stmt)) {
    DeclStmt *declstmt = (DeclStmt *) stmt;

    /* Get Decl object from DeclStmt.  */
    const DeclGroupRef decl_group = declstmt->getDeclGroup();
    for (Decl *decl : decl_group) {
      /* Look into the type for a RecordDecl.  */
      if (ValueDecl *valuedecl = dynamic_cast<ValueDecl *>(decl)) {
        type = valuedecl->getType().getTypePtr();
        Add_Type_And_Depends(type);
      }
    }

    type = nullptr;
  } else if (DeclRefExpr::classof(stmt)) {
    /* Handle global variables and references to an enum.  */
    DeclRefExpr *expr = (DeclRefExpr *) stmt;
    ValueDecl *decl = expr->getDecl();

    if (decl) {
      type = decl->getType().getTypePtr();
      Decl *to_mark = decl;

      /* If the decl is a reference to an enum field then we must add the
         enum declaration instead.  */
      if (EnumConstantDecl *ecdecl = dynamic_cast<EnumConstantDecl *>(decl)) {
        to_mark = EnumTable.Get(ecdecl);
        assert(to_mark && "Reference to EnumDecl not in EnumTable.");
      }

      if (!Is_Decl_Marked(to_mark))
        Add_Decl_And_Prevs(to_mark);
    }

  } else if (Expr::classof(stmt)) {
    Expr *expr = (Expr *) stmt;

    type = expr->getType().getTypePtr();
  }

  if (type) {
    Add_Type_And_Depends(type);
  }

  /* Iterate through the list of all children Statements and repeat the
     process.  */
  clang::Stmt::child_iterator it, it_end;
  for (it = stmt->child_begin(), it_end = stmt->child_end();
      it != it_end; ++it) {

    Stmt *child = *it;
    Mark_Types_In_Function_Body(child);
  }
}

/** FunctionDependencyFinder class methods implementation.  */

FunctionDependencyFinder::FunctionDependencyFinder(ASTUnit *ast,
                                                   std::string const &function)
  : AST(ast),
    EnumTable(AST)
{
  std::vector<std::string> functions{ function };
  Run_Analysis(functions);
}


FunctionDependencyFinder::FunctionDependencyFinder(ASTUnit *ast,
                                                   std::vector<std::string> const &functions)
  : AST(ast),
    EnumTable(AST)
{
  Run_Analysis(functions);
}

/** Add a decl to the Dependencies set and all its previous declarations in the
    AST. A function can have multiple definitions but its body may only be
    defined later.  */
bool FunctionDependencyFinder::Add_Decl_And_Prevs(Decl *decl)
{
  bool inserted = false;
  while (decl) {
    if (!Is_Decl_Marked(decl)) {
      Dependencies.insert(decl);
      inserted = true;
    }
    decl = decl->getPreviousDecl();
  }

  return inserted;
}

/** Pretty print all nodes were marked to output.  */
void FunctionDependencyFinder::Print()
{
  clang::ASTUnit::top_level_iterator it = AST->top_level_begin();
  for (it = AST->top_level_begin(); it != AST->top_level_end(); ++it) {
    Decl *decl = *it;

    if (Is_Decl_Marked(decl)) {
      PrettyPrint::Print_Decl(decl);
    }
  }
}

bool FunctionDependencyFinder::Handle_Array_Size(ValueDecl *decl)
{
  if (decl == nullptr)
    return false;

  /* We can't continue if no SourceManager was provided to the PrettyPrint
     class.  */
  if (PrettyPrint::Get_Source_Manager() == nullptr) {
    return false;
  }

  /* If the type is not an Array Type then there is no point in doing this
     analysis.  */
  const Type *type = decl->getType().getTypePtr();
  if (!type->isConstantArrayType()) {
    return false;
  }


  StringRef str = PrettyPrint::Get_Source_Text(decl->getSourceRange());
  size_t first_openbrk = str.find("[");
  size_t first_closebrk = str.find("]", first_openbrk);
  long str_size = first_closebrk - first_openbrk - 1;

  /* This declaration may be a result of an macro, in which case we won't
     find any [].*/
  if (str_size < 0)
    return false;

  StringRef substr = str.substr(first_openbrk + 1, str_size);

  /* FIXME: Don't assume that the entire substring is the identifier. Instead
     do some tokenization, either by using something similar to strtok or
     implementing a small Recursive-Descent parser on the classical LL(1)
     arithmetic grammar which is able to parse arithmetic expressions.  */
  EnumDecl *enum_decl = EnumTable.Get(substr);
  if (enum_decl && !Is_Decl_Marked(enum_decl)) {
    Add_Decl_And_Prevs(enum_decl);
  }

  return true;
}

void FunctionDependencyFinder::Run_Analysis(std::vector<std::string> const &functions)
{
  /* Step 1: Build the CallGraph.  */
  CallGraph *cg = Build_CallGraph_From_AST(AST);

  /* Step 2: Find all functions that depends from `function`.  */
  Find_Functions_Required(cg, functions);

  /* Step 3: Find all Global variables and Types required by the functions
     that are currently in the Dependencies set..  */
  Compute_Closure();

  delete cg;
}
