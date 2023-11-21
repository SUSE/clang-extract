#include "FunctionDepsFinder.hh"
#include "ClangCompat.hh"
#include "PrettyPrint.hh"
#include "clang/Analysis/CallGraph.h"
#include "clang/Sema/IdentifierResolver.h"
#include "clang/AST/ParentMapContext.h"

#define PProcessor (AST->getPreprocessor())

bool Is_Builtin_Decl(const Decl *decl)
{
  const NamedDecl *ndecl = dyn_cast<const NamedDecl>(decl);
  if (ndecl) {
    StringRef name = ndecl->getName();
    if (name.starts_with("__builtin_")) {
      return true;
    }

    if (name.starts_with("__compiletime_assert_")) {
      return true;
    }
  }

  return false;
}

void FunctionDependencyFinder::Find_Functions_Required(
    std::vector<std::string> const &funcnames)
{
  assert(funcnames.size() > 0);

  ASTUnit::top_level_iterator it;
  for (it = AST->top_level_begin(); it != AST->top_level_end(); ++it) {
    /* Get Declaration as a FunctionDecl to query its name.  */
    FunctionDecl *decl = dynamic_cast<FunctionDecl *>(*it);

    if (!decl)
      continue;

    decl = decl->getDefinition();

    if (!decl)
      continue;

    /* Find the function which name matches our funcname.  */
    for (const std::string &funcname : funcnames) {
      if (decl->getNameAsString() == funcname) {
        /* Now analyze the function to compute its closure.  */
        Mark_Required_Functions(decl);
      }
    }
  }

  /* Handle the corner case where an global array was declared as

    enum {
      const1 = 1,
    }

    int var[cost1];

    in which clang will unfortunatelly expands const1 to 1 and lose the
    EnumConstantDecl in this case, forcing us to reparse this declaration.  */
  for (Decl *decl : Dependencies) {
    VarDecl *vardecl = dynamic_cast<VarDecl *>(decl);
    if (vardecl) {
      Handle_Array_Size(vardecl);
    }
  }
}


bool FunctionDependencyFinder::Mark_Required_Functions(FunctionDecl *decl)
{
  if (!decl)
    return false;

  /* If the function is already in the FunctionDependency set, then we can
     quickly return because this node was already analyzed.  */
  if (Is_Decl_Marked(decl))
    return false;

  if (decl->getBuiltinID() != 0) {
    /* Do not redeclare compiler builtin functions.  */
    return false;
  }

  bool ret;

  /* In case this function has a body, we mark its body as well.  */
  ret = Add_Decl_And_Prevs(decl->isStatic() ? decl->getDefinition() : decl);

  /* Analyze the return type.  */
  ret |= Add_Type_And_Depends(ClangCompat::getTypePtr(decl->getReturnType()));

  /* Analyze the function parameters.  */
  for (ParmVarDecl *param : decl->parameters()) {
    ret |= Add_Type_And_Depends(ClangCompat::getTypePtr(param->getOriginalType()));
  }

  /* Analyze body, which will add functions in a DFS fashion.  */
  if (decl->hasBody() || decl->isStatic())
    ret |= Mark_Types_In_Function_Body(decl->getBody());

  return ret;
}

bool FunctionDependencyFinder::Add_Type_And_Depends(const Type *type)
{
  if (!type)
    return false;

  bool inserted = false;

  if (TypeOfExprType::classof(type)) {
    const TypeOfExprType *t = type->getAs<const TypeOfExprType>();
    Mark_Types_In_Function_Body(t->getUnderlyingExpr());
  }

  /* Check type to the correct type and handle it accordingly.  */
  if (TagType::classof(type)) {
    /* Handle case where type is a struct, union, or enum.  */

    const TagType *t = type->getAs<const TagType>();
    return Handle_TypeDecl(t->getAsTagDecl());
  }

  if (isa<const TypedefType>(type)) {
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

    if (inserted) {
      Add_Type_And_Depends(ClangCompat::getTypePtr(t->desugar()));
    }

    return inserted;
  }

  if (isa<const FunctionProtoType>(type)) {
    /* Handle function prototypes.  Can appear in typedefs or struct keys
       in ways like this:

        typedef int (*GEN_SESSION_CB)(int *, unsigned char *, unsigned int *);

       */
    const FunctionProtoType *prototype = type->getAs<const FunctionProtoType>();
    int n = prototype->getNumParams();

    /* Add function return type.  */
    inserted |= Add_Type_And_Depends(ClangCompat::getTypePtr(prototype->getReturnType()));

    /* Add function parameters.  */
    for (int i = 0; i < n; i++) {
      inserted |= Add_Type_And_Depends(ClangCompat::getTypePtr(prototype->getParamType(i)));
    }

    return inserted;
  }

  /* The `isElaboratedTypeSpecifier` seems to reject some cases where the type
     is indeed an ElaboratedType.  */
  if (type->isElaboratedTypeSpecifier() || isa<ElaboratedType>(type)) {
    const ElaboratedType *t = type->getAs<const ElaboratedType>();
    if (t != nullptr) {
      /* ElaboratedType encode information about template specification
         (see c++-8 testcase).  */
      NestedNameSpecifier *nns = t->getQualifier();
      if (nns != nullptr) {
        Add_Type_And_Depends(nns->getAsType());
      }
    }

    return Add_Type_And_Depends(ClangCompat::getTypePtr(t->desugar()));
  }

  /* Parse the template arguments.  */
  if (isa<TemplateSpecializationType>(type)) {
    const TemplateSpecializationType *t =
      type->getAs<const TemplateSpecializationType>();

    for (TemplateArgument arg : t->template_arguments()) {
      Add_Type_And_Depends(ClangCompat::getTypePtr(arg.getAsType()));
    }

    return Add_Type_And_Depends(ClangCompat::getTypePtr(t->desugar()));
  }

  /* Deduced template specialization.  */
  if (isa<DeducedTemplateSpecializationType>(type)) {
    const DeducedTemplateSpecializationType *t =
              type->getAs<const DeducedTemplateSpecializationType>();

    return Add_Type_And_Depends(ClangCompat::getTypePtr(t->getDeducedType()));
  }

  if (isa<const ParenType>(type)) {
    const ParenType *t = type->getAs<const ParenType>();
    return Add_Type_And_Depends(ClangCompat::getTypePtr(t->desugar()));
  }

  /* Typeof(x).  */
  if (isa<const TypeOfType>(type)) {
    const TypeOfType *t = type->getAs<const TypeOfType>();
    return Add_Type_And_Depends(ClangCompat::getTypePtr(t->desugar()));
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

  /* Be careful with anon structs declarations.  */
  RecordDecl *rdecl = dynamic_cast<RecordDecl *>(decl);
  if (rdecl) {
    if (TypedefNameDecl *typedefdecl = rdecl->getTypedefNameForAnonDecl()) {
      /* In case the record decl was declared in an anon typedef, as in
       *
       * typedef struct { int x; } X;
       *
       * then insert the typedef instead, not the anon struct declaration
       */

      decl = typedefdecl;
    }
  } else if (TypedefNameDecl *tdecl = dynamic_cast<TypedefNameDecl *>(decl)) {
    /* In case the given decl is a typedef, we must be careful with the
     * following case:
     *
     *  typedef unsigned long pteval_t;
     *  typedef struct { pteval_t pte; } pte_t;
     *
     * Assuming that was given the node for the pte_t typedef, we must
     * recursively look into the declarations inside the struct to find
     * if there is a reference to a type that needs to be included in the
     * closure.
     */
    rdecl = dynamic_cast<RecordDecl *>(tdecl->getAnonDeclWithTypedefName());
    if (rdecl == nullptr) {
      /* On C++ there is a possibility that getAnonDeclWithTypedefType returns
         NULL, yet it specifies an type to a node which needs to be inserted,
         for instance on template specification.  In this case, this type is
         a ElaboratedType and we need to get it through its NestedNameSpecifier.  */
      Add_Type_And_Depends(ClangCompat::getTypePtr(tdecl->getUnderlyingType()));
    }
  }

  /* Be careful with RecordTypes comming from templates.  */
  if (ClassTemplateSpecializationDecl *templtspecial =
      dynamic_cast<ClassTemplateSpecializationDecl *>(decl)) {

    /* Handle the template arguments.  */
    const TemplateArgumentList &list = templtspecial->getTemplateArgs();
    for (unsigned i = 0; i < list.size(); i++) {
       const Type *t = ClangCompat::getTypePtr(list[i].getAsType());
       Add_Type_And_Depends(t);
    }

    if (ClassTemplateDecl *templatedecl = templtspecial->getSpecializedTemplate()) {
      /* This record declaration comes from a template declaration, so we must
         add it as well.  */
      Add_Decl_And_Prevs(templatedecl);
    }
  }

  /* If decl was already inserted then quickly return.  */
  if (Is_Decl_Marked(decl))
    return false;

  /* Add current decl.  */
  inserted = Add_Decl_And_Prevs(decl);

  if (rdecl) {
    /* In C++, RecordDecl may have inheritance.  Handle it here.  */
    if (CXXRecordDecl *cxxrdecl = dynamic_cast<CXXRecordDecl *>(rdecl)) {
      /* For some reason, trying to iterate on all base classes of a
         bodyless declaration results in a crash from clang's side.  */
      if (cxxrdecl->hasDefinition()) {
        for (CXXBaseSpecifier base : cxxrdecl->bases()) {
          const Type *type = ClangCompat::getTypePtr(base.getType());
          Add_Type_And_Depends(type);
        }
      }
    }

    /* RecordDecl may have nested records, for example:

      struct A {
        int a;
      };

      struct B {
        struct A a;
        int b;
      };

      in this case we have to recursively analyze the nested types as well.  */
    Handle_DeclContext(rdecl);
  }

  return inserted;
}

bool FunctionDependencyFinder::Handle_DeclContext(DeclContext *decl)
{
  bool inserted = false;

  for (Decl *d : decl->decls()) {
    /* Handle fields in case of structs and classes.  */
    if (ValueDecl *valuedecl = dynamic_cast<ValueDecl *>(d)) {
      Handle_Array_Size(valuedecl);
      Add_Type_And_Depends(ClangCompat::getTypePtr(valuedecl->getType()));
    } else if (CXXMethodDecl *method = dynamic_cast<CXXMethodDecl *>(d)) {
      /* Handle C++ declarations that can be done inside structs and classes.  */
      Mark_Required_Functions(method);
    } else if (TypeDecl *typedecl = dynamic_cast<TypeDecl *>(d)) {
      Handle_TypeDecl(typedecl);
    }
  }

  return inserted;
}

bool FunctionDependencyFinder::Handle_Decl(Decl *decl)
{
  if (TypeDecl *d = dyn_cast<TypeDecl>(decl)) {
    return Handle_TypeDecl(d);
  }

  if (EnumDecl *d = dyn_cast<EnumDecl>(decl)) {
    return Handle_EnumDecl(d);
  }

  if (FunctionDecl *d = dyn_cast<FunctionDecl>(decl)) {
    return Mark_Required_Functions(d);
  }

  if (VarDecl *d = dyn_cast<VarDecl>(decl)) {
    bool ret = Mark_Types_In_Function_Body(d->getInit());
    ret |= Add_Type_And_Depends(ClangCompat::getTypePtr(d->getType()));
    return ret;
  }

  if (StaticAssertDecl *d = dyn_cast<StaticAssertDecl>(decl)) {
    bool r1 = Mark_Types_In_Function_Body(d->getAssertExpr());
    bool r2 = Mark_Types_In_Function_Body(d->getMessage());
    return r1 || r2;
  }

  return Add_Decl_And_Prevs(decl);
}

bool FunctionDependencyFinder::Handle_EnumDecl(EnumDecl *decl) {
  /* Mark the enum decl first to avoid problems with the following enums:

    enum {
      CONST0,
      CONST1 = CONST0,
    };
  */
  Add_Decl_And_Prevs(decl);

  for (EnumConstantDecl *enum_const : decl->enumerators()) {
    Expr *expr = enum_const->getInitExpr();
    Mark_Types_In_Function_Body(expr);
  }

  return true;
}

bool FunctionDependencyFinder::Mark_Types_In_Function_Body(Stmt *stmt)
{
  if (!stmt)
    return false;

  bool ret = false;

  /* For some akward reason we can't use dynamic_cast<>() on Stmt.  So
     we poke in this classof which seems to work.  */
  const Type *type = nullptr;

  if (DeclStmt::classof(stmt)) {
    DeclStmt *declstmt = (DeclStmt *)stmt;

    /* Get Decl object from DeclStmt.  */
    const DeclGroupRef decl_group = declstmt->getDeclGroup();
    for (Decl *decl : decl_group) {
      /* Look into the type for a RecordDecl.  */
      if (ValueDecl *valuedecl = dynamic_cast<ValueDecl *>(decl)) {
        type = ClangCompat::getTypePtr(valuedecl->getType());
        ret |= Add_Type_And_Depends(type);
      }
    }

    type = nullptr;
  } else if (DeclRefExpr::classof(stmt)) {
    /* Handle global variables and references to an enum.  */
    DeclRefExpr *expr = (DeclRefExpr *)stmt;

    ValueDecl *decl = expr->getDecl();

    if (decl) {
      type = ClangCompat::getTypePtr(decl->getType());
      Decl *to_mark = decl;

      /* If the decl is a reference to an enum field then we must add the
         enum declaration instead.  */
      if (EnumConstantDecl *ecdecl = dynamic_cast<EnumConstantDecl *>(decl)) {
        EnumDecl *enum_decl = EnumTable.Get(ecdecl);
        assert(to_mark && "Reference to EnumDecl not in EnumTable.");

        /* Analyze the original enum to find references to other enum
           constants.  */
        if (enum_decl && !Is_Decl_Marked(enum_decl)) {
          ret |= Handle_EnumDecl(enum_decl);
        }
      } else if (FunctionDecl *fundecl = dynamic_cast<FunctionDecl *>(decl)) {
        /* This is a reference to a function. We have to analyze it recursively.
         */
        ret |= Mark_Required_Functions(fundecl);

        /* Get immediate parent from function.  Required if function is a
           template.  */
        ASTContext &ctx = AST->getASTContext();
        DynTypedNodeList list = ctx.getParents(*fundecl);

        /* Iterate on the list of parents to find the FunctionTemplateDecl and
           mark for output.  */
        for (unsigned i = 0; i < list.size(); i++) {
          const DynTypedNode n = list[i];
          if (const Decl *d = n.get<FunctionTemplateDecl>()) {
            ret |= Add_Decl_And_Prevs((Decl *)(d));
          }
        }

      } else {

        type = ClangCompat::getTypePtr(decl->getType());
        ret |= Add_Type_And_Depends(type);

        if (!Is_Decl_Marked(to_mark)) {
          ret |= Add_Decl_And_Prevs(to_mark);
          if (VarDecl *vardecl = dynamic_cast<VarDecl *>(decl)) {
            /* This is a reference to a global variable.  Look for its initializer symbols.  */
            if (vardecl->hasGlobalStorage() && vardecl->hasInit()) {
              ret |= Mark_Types_In_Function_Body(vardecl->getInit());
            }
            /* Add its type.  */
            ret |= Add_Type_And_Depends(ClangCompat::getTypePtr(vardecl->getType()));
          }
        }
      }
    }

  } else if (AsmStmt::classof(stmt)) {
    /* Handle ASM statements like (see asm-1.c):
        asm volatile ("mov %1, %0\n\t"
                                   : "=r"(dest)
                                   : "i" (__builtin_offsetof(Head, y))
                                   : );  */

    AsmStmt *asmstmt = (AsmStmt *)stmt;
    unsigned num_inputs = asmstmt->getNumInputs();
    unsigned num_outputs = asmstmt->getNumOutputs();

    for (unsigned i = 0; i < num_inputs; i++) {
      Expr *expr = (Expr *)asmstmt->getInputExpr(i);
      ret |= Mark_Types_In_Function_Body(expr);
    }

    for (unsigned i = 0; i < num_outputs; i++) {
      Expr *expr = (Expr *)asmstmt->getOutputExpr(i);
      ret |= Mark_Types_In_Function_Body(expr);
    }

  } else if (OffsetOfExpr::classof(stmt)) {
    /* Handle the following case:
     *
     * typedef struct {
     *   int x;
     *   int y;
     * } Head;
     *
     * int x = __builtin_offsetof(Head, y);
     */
    OffsetOfExpr *expr = (OffsetOfExpr *)stmt;
    unsigned num_components = expr->getNumComponents();
    for (unsigned i = 0; i < num_components; i++) {
      /* Get the RecordDecl referenced in the builtin_offsetof and add to the
         dependency list.  */
      const OffsetOfNode &component = expr->getComponent(i);
      if (component.getKind() == OffsetOfNode::Field) {
        const FieldDecl *field = component.getField();
        RecordDecl *record = (RecordDecl *)field->getParent();
        ret |= Handle_TypeDecl(record);
      }
    }
  } else if (UnaryExprOrTypeTraitExpr::classof(stmt)) {
    UnaryExprOrTypeTraitExpr *expr = (UnaryExprOrTypeTraitExpr *)stmt;
    type = ClangCompat::getTypePtr(expr->getTypeOfArgument());

  } else if (TypeTraitExpr::classof(stmt)) {
    const TypeTraitExpr *expr = (const TypeTraitExpr*)stmt;

    for (TypeSourceInfo *info : expr->getArgs()) {
      const Type *t = ClangCompat::getTypePtr(info->getType());
      ret |= Add_Type_And_Depends(t);
    }
  } else if (Expr::classof(stmt)) {
    Expr *expr = (Expr *)stmt;

    type = ClangCompat::getTypePtr(expr->getType());
  }

  if (type) {
    ret |= Add_Type_And_Depends(type);
  }

  /* Iterate through the list of all children Statements and repeat the
     process.  */
  clang::Stmt::child_iterator it, it_end;
  for (it = stmt->child_begin(), it_end = stmt->child_end(); it != it_end;
       ++it) {

    Stmt *child = *it;
    ret |= Mark_Types_In_Function_Body(child);
  }

  return ret;
}

/** FunctionDependencyFinder class methods implementation.  */
FunctionDependencyFinder::FunctionDependencyFinder(PassManager::Context *ctx)
    : AST(ctx->AST.get()),
      EnumTable(AST),
      MW(AST->getPreprocessor()),
      IT(AST->getPreprocessor(), ctx->HeadersToExpand),
      KeepIncludes(ctx->KeepIncludes)
{
  Run_Analysis(ctx->FuncExtractNames);
}

/** Add a decl to the Dependencies set and all its previous declarations in the
    AST. A function can have multiple definitions but its body may only be
    defined later.  */
bool FunctionDependencyFinder::Add_Decl_And_Prevs(Decl *decl)
{
  bool inserted = false;

  /* Do not insert builtin decls.  */
  if (Is_Builtin_Decl(decl)) {
    return false;
  }

  while (decl) {
    if (!Is_Decl_Marked(decl)) {
      Dependencies.insert(decl);
      inserted = true;
    }


    /* getPrevious seems broken.  When trying to add `struct key;` from kernel
       6.6-rc7 defined in `include/linux/key.h:33`, getPrevious returns
       `task_struct` which is nonsense: it is not by any means a child of
       struct key.  Hence we need to open a bug in clang about it (but first
       check if it was fixed in llvm-17 or llvm-18).  See issue #12.  */
#if 0
    ASTContext &ctx = AST->getASTContext();
    DynTypedNodeList list = ctx.getParents(*decl);

    /* Iterate on the list of parents to find the FunctionTemplateDecl and
       mark for output.  */
    for (unsigned i = 0; i < list.size(); i++) {
      const DynTypedNode n = list[i];
      Decl *d = (Decl *) n.get<const Decl>();
      if (d != nullptr) {
        inserted |= Add_Decl_And_Prevs(d);
      }
    }
#endif

    decl = decl->getPreviousDecl();
  }
  return inserted;
}

void FunctionDependencyFinder::Print(void)
{
  RecursivePrint(AST, Dependencies, IT, KeepIncludes).Print();
}

bool FunctionDependencyFinder::Handle_Array_Size(ValueDecl *decl) {
  if (decl == nullptr)
    return false;

  /* If the type is not an Array Type then there is no point in doing this
     analysis.  */
  const Type *type = ClangCompat::getTypePtr(decl->getType());
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

/** Set containing the macros which were already analyzed.  */
static std::unordered_set<MacroInfo *> AnalyzedMacros;
static bool Already_Analyzed(MacroInfo *info) {
  return AnalyzedMacros.find(info) != AnalyzedMacros.end();
}

bool FunctionDependencyFinder::Backtrack_Macro_Expansion(
    MacroInfo *info, const SourceLocation &loc) {
  bool inserted = false;

  if (info == nullptr)
    return false;

  /* If this macro were already been analyzed, then quickly return.

     We can not use the MacroDependency set for that because macros
     can reference macros that are later redefined. For example:

     #define A B
     int A;

     #define A C
     int A;.  */

  if (Already_Analyzed(info)) {
    return false;
  }

  AnalyzedMacros.insert(info);

  /* If this MacroInfo object with the macro contents wasn't marked for output
     then mark it now.  */
  if (!Is_Macro_Marked(info)) {
    inserted = true;
  }

  /* We can not quickly return if the macro is already marked.  Other macros
     which this macro depends on may have been redefined in meanwhile, and
     we don't implement some dependency tracking so far, so redo the analysis.
   */

  auto it = info->tokens_begin(); /* Use auto here as the it type changes
                                     according to clang version.  */
  /* Iterate on the expansion tokens of this macro to find if it references
     other macros.  */
  for (; it != info->tokens_end(); ++it) {
    const Token *tok = it;
    const IdentifierInfo *tok_id = tok->getIdentifierInfo();

    if (tok_id != nullptr) {
      /* We must be careful to not confuse tokens which are function-like macro
         arguments with other macors.  Example:

         #define MAX(a, b) ((a) > (b) ? (a) : (b))
         #define a 9

         We should not add `a` as macro when analyzing MAX once it is clearly an
         argument of the macro, not a reference to other symbol.  */
      if (!MacroWalker::Is_Identifier_Macro_Argument(info, tok_id)) {
        MacroInfo *maybe_macro =
            MW.Get_Macro_Info(tok->getIdentifierInfo(), loc);

        /* If this token is actually a name to a declared macro, then analyze it
           as well.  */
        if (maybe_macro) {
          inserted |= Backtrack_Macro_Expansion(maybe_macro, loc);

          /* If the token is a name to a constant declared in an enum, then add
             the enum as well.  */
        } else if (EnumDecl *edecl =
                       EnumTable.Get(tok->getIdentifierInfo()->getName())) {
          if (edecl && !Is_Decl_Marked(edecl)) {
            Handle_EnumDecl(edecl);
          }
        }
      }
    }
  }

  return inserted;
}

bool FunctionDependencyFinder::Backtrack_Macro_Expansion(
    MacroExpansion *macroexp) {
  MacroInfo *info = MW.Get_Macro_Info(macroexp);
  return Backtrack_Macro_Expansion(info, macroexp->getSourceRange().getBegin());
}

void FunctionDependencyFinder::Remove_Redundant_Decls(void) {
  ASTUnit::top_level_iterator it;

  for (it = AST->top_level_begin(); it != AST->top_level_end(); it++) {

    /* Handle the case where a enum is declared as:

        typedef enum {
          LEFT,
          RIGHT
        } Hand;

        which is then broken as

        enum {
          LEFT,
          RIGHT
        };

        typedef enum Hand Hand;

        This one will remove the first enum and only mark the typedef
        one because the location tracking will output the typedef
        correctly.  */
    if (TypedefDecl *decl = dynamic_cast<TypedefDecl *>(*it)) {
      SourceRange range = decl->getSourceRange();

      const Type *type = decl->getTypeForDecl();
      if (type) {
        TagDecl *typedecl = type->getAsTagDecl();

        if (typedecl && Is_Decl_Marked(typedecl)) {
          SourceRange type_range = typedecl->getSourceRange();

          /* Check if the strings regarding an decl empty. In that case we
             can not delete the decl from list.  */
          if (PrettyPrint::Get_Source_Text(range) != "" &&
              PrettyPrint::Get_Source_Text(type_range) != "") {

            /* Using .fullyContains() fails in some declarations.  */
            if (PrettyPrint::Contains_From_LineCol(range, type_range)) {
              Dependencies.erase(typedecl);
            }
          }
        }
      }
    }
    /* Handle the case where an enum is declared as:
        extern enum system_states {
          SYSTEM_BOOTING,
          SYSTEM_SCHEDULING,
          SYSTEM_FREEING_INITMEM,
          SYSTEM_RUNNING,
          SYSTEM_HALT,
          SYSTEM_POWER_OFF,
          SYSTEM_RESTART,
          SYSTEM_SUSPEND,
        } system_state;

      In which will be broken down into:

        enum system_states {
          ...
        };

        enum system_states system_state;

        this will remove the first enum declaration because the location
        tracking will correctly include the enum system_states.  */

    else if (DeclaratorDecl *decl = dynamic_cast<DeclaratorDecl *>(*it)) {
      SourceRange range = decl->getSourceRange();

      const Type *type = ClangCompat::getTypePtr(decl->getType());
      TagDecl *typedecl = type->getAsTagDecl();
      if (typedecl && Is_Decl_Marked(typedecl)) {
        SourceRange type_range = typedecl->getSourceRange();

        /* Using .fullyContains() fails in some declarations.  */
        if (PrettyPrint::Contains_From_LineCol(range, type_range)) {
          Dependencies.erase(typedecl);
        }
      }
    }
  }
}

void FunctionDependencyFinder::Include_Enum_Constants_Referenced_By_Macros(
    void) {
  /* Ensure that PreprocessingRecord is avaialable.  */
  PreprocessingRecord *rec = PProcessor.getPreprocessingRecord();
  assert(rec && "PreprocessingRecord wasn't generated");

  clang::PreprocessingRecord::iterator macro_it;
  for (macro_it = rec->begin(); macro_it != rec->end(); macro_it++) {
    if (MacroDefinitionRecord *macroexp =
            dyn_cast<MacroDefinitionRecord>(*macro_it)) {
      MacroInfo *info = MW.Get_Macro_Info(macroexp);

      if (Is_Macro_Marked(info)) {
        Backtrack_Macro_Expansion(info, macroexp->getSourceRange().getBegin());
      }
    }
  }
}

void FunctionDependencyFinder::Insert_Decls_From_Non_Expanded_Includes(void)
{
  ASTUnit::top_level_iterator it;
  for (it = AST->top_level_begin(); it != AST->top_level_end(); ++it) {
    Decl *decl = *it;

    const SourceLocation &loc = decl->getLocation();
    IncludeNode *node = IT.Get(loc);

    if (node && node->Has_Parent_Marked_For_Output()) {
      Handle_Decl(decl);
    }
  }
}

void FunctionDependencyFinder::Run_Analysis(std::vector<std::string> const &functions)
{
  /* Step 1: Compute the closure.  */
  Find_Functions_Required(functions);

  /* Step 2: Find enum constants which are referenced by macros.  */
  Include_Enum_Constants_Referenced_By_Macros();

  /* Step 3: Expand the closure to include Decls in non-expanded includes.  */
  if (KeepIncludes) {
    /* TODO: Guarding this with KeepIncludes should actually not be necessary,
       but we still have bugs when processing the kernel and the closure is not
       perfect.  By not adding those declarations we avoid hitting cases where
       it would hang because of a declaration that would be removed in the
       future anyway.  */
    Insert_Decls_From_Non_Expanded_Includes();
  }

  /* Step 4: Remove any declaration that may have been declared twice.  */
  Remove_Redundant_Decls();
}
