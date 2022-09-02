#include "FunctionDepsFinder.hh"
#include "PrettyPrint.hh"
#include "clang/Analysis/CallGraph.h"

/** Used in prototyping.  */
static bool Is_Record_Reachable(Decl *decl)
{
  RecordDecl *recdecl = dynamic_cast<RecordDecl *>(decl);
  while (recdecl) {
    if (recdecl && recdecl->isReferenced()) {
      return true;
    }
    recdecl = recdecl->getPreviousDecl();
  }

  return false;
}

/** Used in prototyping.  */
static bool Is_Typedef_Reachable(Decl *decl)
{
  TypedefDecl *typedecl = dynamic_cast<TypedefDecl *>(decl);
  if (typedecl /*&& typedecl->isReferenced()*/) {
    return true;
  }
  return false;
}

/** Used in prototyping.  */
static bool Is_Global_Var_Reachable(Decl *decl)
{
  VarDecl *vardecl = dynamic_cast<VarDecl *>(decl);
  if (vardecl && vardecl->isReferenced()) {
    return true;
  }
  return false;
}


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
static CallGraph *Build_CallGraph_From_AST(ASTUnit *ast)
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
            (CallGraph *cg, std::string const& funcname)
{
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

      /* Return now, we don't need to check for any other node here.  */
      return;
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

void FunctionDependencyFinder::Find_Records_Required(void)
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

  /* Finally iterate through all function definitions and trigger the
     RecordDecl dependency tracker.  */
  for (size_t i = 0; i < n; i++) {
    FunctionDecl *decl = dependencies[i]->getAsFunction();
    Mark_Records_In_Function_Body(decl->getBody());
  }
}

static const Type *PointerType_Deference(const Type *type)
{
  if (type->isAnyPointerType()) {
    return PointerType_Deference(type->getPointeeType().getTypePtr());
  } else if (type->isElaboratedTypeSpecifier()) {
    const ElaboratedType *t = (const ElaboratedType *) type;
    return PointerType_Deference(t->desugar().getTypePtr());
  } else if (type->isArrayType()) {
    const ArrayType *t = (const ArrayType *) type;
    return PointerType_Deference(t->getElementType().getTypePtr());
  }

  return type;
}

void FunctionDependencyFinder::Mark_Records_In_Function_Body(Stmt *stmt)
{
  if (!stmt)
    return;

  /* For some akward reason we can't use dynamic_cast<>() on Stmt.  So
     we poke in this classof which seems to work.  */
  DeclStmt *declstmt = nullptr;
  TagDecl *tagdecl = nullptr;

  if (DeclStmt::classof(stmt)) {
    //Print_Stmt(stmt);
    declstmt = (DeclStmt *) stmt;

    /* Get Decl object from DeclStmt.  */
    Decl *decl = declstmt->getSingleDecl();

    /* Look into the type for a RecordDecl.  */
    ValueDecl *valuedecl = dynamic_cast<ValueDecl *>(decl);
    const Type *type = PointerType_Deference(valuedecl->getType().getTypePtr());
    tagdecl = type->getAsTagDecl();

  } else if (Expr::classof(stmt)) {
    Expr *expr = (Expr *) stmt;

    const Type *type = PointerType_Deference(expr->getType().getTypePtr());
    tagdecl = type->getAsTagDecl();
  }

  /* If this is a valid RecordDecl or enum (for example, the type is not a 
     primitive type such as `int`, then we mark this Decl in the Dependencies
     set.  */
  if (tagdecl) {
    Add_TagDecl_And_Nested(tagdecl);
  } 

  /* Iterate through the list of all children Statements and repeat the
     process.  */
  clang::Stmt::child_iterator it, it_end;
  for (it = stmt->child_begin(), it_end = stmt->child_end();
      it != it_end; ++it) {

    Stmt *child = *it;
    Mark_Records_In_Function_Body(child);
  }
}

/** FunctionDependencyFinder class methods implementation.  */

FunctionDependencyFinder::FunctionDependencyFinder(std::unique_ptr<ASTUnit> ast,
                                                   std::string const &function)
{
  AST = std::move(ast);
  Run_Analysis(function);
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

bool FunctionDependencyFinder::Add_TagDecl_And_Nested(TagDecl *decl)
{
  bool inserted = false;

  if (!decl)
    return false;

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
      const Type *type = PointerType_Deference(valuedecl->getType().getTypePtr());
      TagDecl *tagdecl = type->getAsTagDecl();

      inserted |= Add_TagDecl_And_Nested(tagdecl);
    }
  }

  return inserted;
}

/** Pretty print all nodes were marked to output.  */
void FunctionDependencyFinder::Print()
{
  clang::ASTUnit::top_level_iterator it;
  for (it = AST->top_level_begin(); it != AST->top_level_end(); ++it) {
    Decl *decl = *it;
//#define ECHO_FILE
#ifdef ECHO_FILE
    PrettyPrint::Print_Decl(decl);
#else
    /*if (Is_Record_Reachable(decl)) {
      PrettyPrint::Print_Decl(decl);
    } else if (Is_Typedef_Reachable(decl)) {
      PrettyPrint::Print_Decl(decl);
    } else */if (Is_Global_Var_Reachable(decl)) {
      PrettyPrint::Print_Decl(decl);
    } else if (Is_Decl_Marked(decl)) {
      PrettyPrint::Print_Decl(decl);
    }
#endif
  }
}

void FunctionDependencyFinder::Run_Analysis(std::string const &function)
{
  /* Step 1: Build the CallGraph.  */
  CallGraph *cg = Build_CallGraph_From_AST(AST.get());

  /* Step 2: Find all functions that depends from `function`.  */
  Find_Functions_Required(cg, function);

  /* Step 3: Find all RecordDecls that is reachable from the found functions.  */
  Find_Records_Required();

  delete cg;
}
