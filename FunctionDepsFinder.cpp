#include "FunctionDepsFinder.hh"
#include "ClangCompat.hh"
#include "PrettyPrint.hh"
#include "clang/Analysis/CallGraph.h"
#include "clang/Sema/IdentifierResolver.h"

#define PProcessor (AST->getPreprocessor())

MacroIterator FunctionDependencyFinder::Get_Macro_Iterator(void) {
  MacroIterator it;
  it.macro_it = PProcessor.getPreprocessingRecord()->begin();
  it.undef_it = 0;
  return it;
}

void FunctionDependencyFinder::Skip_Macros_Until(MacroIterator &it,
                                                 const SourceLocation &loc) {
  Print_Macros_Until(it, loc, /*print = */ false);
}

void FunctionDependencyFinder::Print_Macros_Until(MacroIterator &it,
                                                  const SourceLocation &loc,
                                                  bool print) {
  PreprocessingRecord *rec = PProcessor.getPreprocessingRecord();

  /* Ensure that PreprocessingRecord is avaialable.  */
  assert(rec && "PreprocessingRecord wasn't generated");

  SourceLocation curr_loc;

  /* We have to iterate in both the preprocessor record and the marked undef
     vector to find which one we should print based on the location.  This is
     somewhat similar to how mergesort merge two sorted vectors, if this
     somehow helps to understand this code.  */
  while (true) {
    PreprocessedEntity *e =
        (it.macro_it == rec->end()) ? nullptr : *it.macro_it;
    MacroDirective *undef = nullptr;
    bool should_print_undef = false;

    /* If we still have items to iterate on the preprocessing record.  */
    if (e) {
      curr_loc = e->getSourceRange().getBegin();
    }

    /* If we still have itens to iterate on the Undef vector.  */
    if (it.undef_it < NeedsUndef.size()) {
      undef = NeedsUndef[it.undef_it];
      SourceLocation undef_loc = undef->getDefinition().getUndefLocation();

      /* Now find out which one comes first.  */
      if (!e || PrettyPrint::Is_Before(undef_loc, curr_loc)) {
        should_print_undef = true;
        curr_loc = undef_loc;
      }
    }

    /* Check if we did not past the loc marker where we should stop printing. */
    if (curr_loc.isValid() && PrettyPrint::Is_Before(curr_loc, loc)) {
      if (should_print_undef) {
        if (print)
          PrettyPrint::Print_Macro_Undef(undef);
        it.undef_it++;
      } else if (e) {
        MacroDefinitionRecord *entity = dyn_cast<MacroDefinitionRecord>(e);
        if (print && entity && Is_Macro_Marked(MW.Get_Macro_Info(entity))) {
          PrettyPrint::Print_Macro_Def(entity);
        }
        it.macro_it++;
      } else {
        break;
      }
    } else {
      break;
    }
  }
}

void FunctionDependencyFinder::Print_Remaining_Macros(MacroIterator &it) {
  PreprocessingRecord *rec = PProcessor.getPreprocessingRecord();

  /* Ensure that PreprocessingRecord is avaialable.  */
  assert(rec && "PreprocessingRecord wasn't generated");

  SourceLocation curr_loc;

  /* We have to iterate in both the preprocessor record and the marked undef
     vector to find which one we should print based on the location.  This is
     somewhat similar to how mergesort merge two sorted vectors, if this
     somehow helps to understand this code.  */
  while (true) {
    PreprocessedEntity *e =
        (it.macro_it == rec->end()) ? nullptr : *it.macro_it;
    MacroDirective *undef = nullptr;
    bool should_print_undef = false;

    /* If we still have items to iterate on the preprocessing record.  */
    if (e) {
      curr_loc = e->getSourceRange().getBegin();
    }

    /* If we still have itens to iterate on the Undef vector.  */
    if (it.undef_it < NeedsUndef.size()) {
      undef = NeedsUndef[it.undef_it];
      SourceLocation undef_loc = undef->getDefinition().getUndefLocation();

      /* Now find out which one comes first.  */
      if (!e || PrettyPrint::Is_Before(undef_loc, curr_loc)) {
        should_print_undef = true;
        curr_loc = undef_loc;
      }
    }

    /* Check if we did not past the loc marker where we should stop printing. */
    if (should_print_undef) {
      PrettyPrint::Print_Macro_Undef(undef);
      it.undef_it++;
    } else if (e) {
      MacroDefinitionRecord *entity = dyn_cast<MacroDefinitionRecord>(e);
      if (entity && Is_Macro_Marked(MW.Get_Macro_Info(entity))) {
        PrettyPrint::Print_Macro_Def(entity);
      }
      it.macro_it++;
    } else {
      break;
    }
  }
}

void FunctionDependencyFinder::Find_Functions_Required(
    std::vector<std::string> const &funcnames) {
  assert(funcnames.size() > 0);

  ASTUnit::top_level_iterator it;
  for (it = AST->top_level_begin(); it != AST->top_level_end(); ++it) {
    /* Get Declaration as a FunctionDecl to query its name.  */
    FunctionDecl *decl = dynamic_cast<FunctionDecl *>(*it);

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

void FunctionDependencyFinder::Mark_Required_Functions(FunctionDecl *decl) {
  if (!decl)
    return;

  /* If the function is already in the FunctionDependency set, then we can
     quickly return because this node was already analyzed.  */
  if (Is_Decl_Marked(decl))
    return;

  /* In case this function has a body, we mark its body as well.  */
  Add_Decl_And_Prevs(decl->hasBody() ? decl->getDefinition() : decl);

  /* Analyze body, which will add functions in a DFS fashion.  */
  Mark_Types_In_Function_Body(decl->getBody());
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

  if (type->isTypedefNameType()) {
    /* Handle case where type is a typedef.  */

    const TypedefType *t = ClangCompat::Get_Type_As_TypedefType(type);
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

  /* The `isElaboratedTypeSpecifier` seems to reject some cases where the type
     is indeed an ElaboratedType.  */
  if (type->isElaboratedTypeSpecifier() || isa<ElaboratedType>(type)) {
    /* For some reason clang add some ElaboratedType types, which is not clear
       of their purpose.  But handle them as well.  */

    const ElaboratedType *t = type->getAs<const ElaboratedType>();

    return Add_Type_And_Depends(t->desugar().getTypePtr());
  }

  /* Parse the template arguments.  */
  if (isa<TemplateSpecializationType>(type)) {
    const TemplateSpecializationType *t =
      type->getAs<const TemplateSpecializationType>();

    for (TemplateArgument arg : t->template_arguments()) {
      Add_Type_And_Depends(arg.getAsType().getTypePtr());
    }

    Add_Type_And_Depends(t->desugar().getTypePtr());
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
  }

  /* Be careful with RecordTypes comming from templates.  */
  if (ClassTemplateSpecializationDecl *templtspecial =
      dynamic_cast<ClassTemplateSpecializationDecl *>(decl)) {
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

void FunctionDependencyFinder::Mark_Types_In_Function_Body(Stmt *stmt)
{
  if (!stmt)
    return;

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
        type = valuedecl->getType().getTypePtr();
        Add_Type_And_Depends(type);
      }
    }

    type = nullptr;
  } else if (DeclRefExpr::classof(stmt)) {
    /* Handle global variables and references to an enum.  */
    DeclRefExpr *expr = (DeclRefExpr *)stmt;
    ValueDecl *decl = expr->getDecl();

    if (decl) {
      type = decl->getType().getTypePtr();
      Decl *to_mark = decl;

      /* If the decl is a reference to an enum field then we must add the
         enum declaration instead.  */
      if (EnumConstantDecl *ecdecl = dynamic_cast<EnumConstantDecl *>(decl)) {
        EnumDecl *enum_decl = EnumTable.Get(ecdecl);
        assert(to_mark && "Reference to EnumDecl not in EnumTable.");

        /* Analyze the original enum to find references to other enum
           constants.  */
        if (enum_decl && !Is_Decl_Marked(enum_decl)) {
          Handle_EnumDecl(enum_decl);
        }
      } else if (FunctionDecl *fundecl = dynamic_cast<FunctionDecl *>(decl)) {
        /* This is a reference to a function. We have to analyze it recursively.
         */
        Mark_Required_Functions(fundecl);
      } else {
        type = decl->getType().getTypePtr();
        Add_Type_And_Depends(type);

        if (!Is_Decl_Marked(to_mark))
          Add_Decl_And_Prevs(to_mark);
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

    for (int i = 0; i < num_inputs; i++) {
      Expr *expr = (Expr *)asmstmt->getInputExpr(i);
      Mark_Types_In_Function_Body(expr);
    }

    for (int i = 0; i < num_outputs; i++) {
      Expr *expr = (Expr *)asmstmt->getOutputExpr(i);
      Mark_Types_In_Function_Body(expr);
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
    for (int i = 0; i < num_components; i++) {
      /* Get the RecordDecl referenced in the builtin_offsetof and add to the
         dependency list.  */
      const OffsetOfNode &component = expr->getComponent(i);
      const FieldDecl *field = component.getField();
      RecordDecl *record = (RecordDecl *)field->getParent();
      Handle_TypeDecl(record);
    }
  } else if (Expr::classof(stmt)) {
    Expr *expr = (Expr *)stmt;

    type = expr->getType().getTypePtr();
  }

  if (type) {
    Add_Type_And_Depends(type);
  }

  /* Iterate through the list of all children Statements and repeat the
     process.  */
  clang::Stmt::child_iterator it, it_end;
  for (it = stmt->child_begin(), it_end = stmt->child_end(); it != it_end;
       ++it) {

    Stmt *child = *it;
    Mark_Types_In_Function_Body(child);
  }
}

/** FunctionDependencyFinder class methods implementation.  */

FunctionDependencyFinder::FunctionDependencyFinder(ASTUnit *ast,
                                                   std::string const &function,
                                                   bool closure)
    : AST(ast), EnumTable(AST), MW(AST->getPreprocessor()) {
  std::vector<std::string> functions{function};
  Run_Analysis(functions, closure);
}

FunctionDependencyFinder::FunctionDependencyFinder(
    ASTUnit *ast, std::vector<std::string> const &funcs, bool closure)
    : AST(ast), EnumTable(AST), MW(AST->getPreprocessor()) {
  Run_Analysis(funcs, closure);
}

/** Add a decl to the Dependencies set and all its previous declarations in the
    AST. A function can have multiple definitions but its body may only be
    defined later.  */
bool FunctionDependencyFinder::Add_Decl_And_Prevs(Decl *decl) {
  bool inserted = false;
  while (decl) {
    if (!Is_Decl_Marked(decl)) {
      Dependencies.insert(decl);
      inserted = true;
    }

    DeclContext *context = decl->getDeclContext();
    if (context) {
      Decl *d = cast<Decl>(context);
      if (d && !Is_Decl_Marked(d)) {
        Dependencies.insert(d);
        inserted = true;
      }
    }

    decl = decl->getPreviousDecl();
  }

  return inserted;
}

/** Pretty print all nodes were marked to output.  */
void FunctionDependencyFinder::Print() {
  SourceLocation last_decl_loc = SourceLocation::getFromRawEncoding(0U);
  bool first = true;

  SourceManager *sm = PrettyPrint::Get_Source_Manager();
  bool can_print_macros = false;

  /* We can only print macros if we have a SourceManager.  */
  if (sm != nullptr) {
    can_print_macros = true;
    FileID mainfile = sm->getMainFileID();
    last_decl_loc = sm->getLocForEndOfFile(mainfile);
  }

  MacroIterator macro_it = Get_Macro_Iterator();
  clang::ASTUnit::top_level_iterator it = AST->top_level_begin();
  for (it = AST->top_level_begin(); it != AST->top_level_end(); ++it) {
    Decl *decl = *it;

    if (Is_Decl_Marked(decl)) {
      if (can_print_macros) {
        /* In the first decl we don't have a last source location, hence we have
           to handle this special case.  */
        if (first) {
          Print_Macros_Until(macro_it, decl->getBeginLoc());
          first = false;
        } else {
          /* Macros defined inside a function is printed together with the
             function, so we must skip them.  */
          Skip_Macros_Until(macro_it, last_decl_loc);

          Print_Macros_Until(macro_it, decl->getBeginLoc());
        }
      }
      last_decl_loc = decl->getEndLoc();
      PrettyPrint::Print_Decl(decl);
    }
  }

  if (can_print_macros) {
    Skip_Macros_Until(macro_it, last_decl_loc);
    /* Print remaining macros.  */
    Print_Remaining_Macros(macro_it);
  }
}

bool FunctionDependencyFinder::Handle_Array_Size(ValueDecl *decl) {
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

      const Type *type = decl->getType().getTypePtr();
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

int FunctionDependencyFinder::Populate_Need_Undef(void) {
  int count = 0;
  PreprocessingRecord *rec = PProcessor.getPreprocessingRecord();

  for (PreprocessedEntity *entity : *rec) {
    if (MacroDefinitionRecord *def = dyn_cast<MacroDefinitionRecord>(entity)) {

      MacroDirective *directive = MW.Get_Macro_Directive(def);

      /* If there is no history, then doesn't bother analyzing.  */
      if (directive == nullptr)
        continue;

      /* There is no point in analyzing a macro that wasn't added for output. */
      if (Is_Macro_Marked(directive->getMacroInfo())) {
        SourceLocation undef_loc =
            directive->getDefinition().getUndefLocation();

        /* In case the macro isn't undefined then its location is invalid.  */
        if (undef_loc.isValid()) {
          NeedsUndef.push_back(directive);
          count++;
        }
      }
    }
  }

  return count;
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

void FunctionDependencyFinder::Run_Analysis(
    std::vector<std::string> const &functions, bool closure) {
  if (closure) {
    /* Step 1: Compute the closure.  */
    Find_Functions_Required(functions);
  } else {
    /* Insert every declaration into the dependency list.  */
    for (auto it = AST->top_level_begin(); it != AST->top_level_end(); it++) {
      Dependencies.insert(*it);
    }
  }

  /* Step 2: Find enum constants which are referenced by macros.  */
  Include_Enum_Constants_Referenced_By_Macros();

  /* Step 3: Handle macros that needs to be undefined.  */
  Populate_Need_Undef();

  /* Step 4: Remove any declaration that may have been declared twice.  */
  Remove_Redundant_Decls();
}
