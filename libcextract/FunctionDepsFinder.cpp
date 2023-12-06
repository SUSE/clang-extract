#include <sstream>
#include <vector>
#include "FunctionDepsFinder.hh"
#include "ClangCompat.hh"
#include "PrettyPrint.hh"
#include "Error.hh"
#include "LLVMMisc.hh"

/** Add a decl to the Dependencies set and all its previous declarations in the
    AST. A function can have multiple definitions but its body may only be
    defined later.  */
bool ClosureSet::Add_Decl_And_Prevs(Decl *decl)
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

    decl = decl->getPreviousDecl();
  }
  return inserted;
}

/// AST visitor for computing the closure of given symbol. From clang:
///
/// A class that does preorder or postorder
/// depth-first traversal on the entire Clang AST and visits each node.
///
/// This class performs three distinct tasks:
///   1. traverse the AST (i.e. go to each node);
///   2. at a given node, walk up the class hierarchy, starting from
///      the node's dynamic type, until the top-most class (e.g. Stmt,
///      Decl, or Type) is reached.
///   3. given a (node, class) combination, where 'class' is some base
///      class of the dynamic type of 'node', call a user-overridable
///      function to actually visit the node.
///
/// These tasks are done by three groups of methods, respectively:
///   1. TraverseDecl(Decl *x) does task #1.  It is the entry point
///      for traversing an AST rooted at x.  This method simply
///      dispatches (i.e. forwards) to TraverseFoo(Foo *x) where Foo
///      is the dynamic type of *x, which calls WalkUpFromFoo(x) and
///      then recursively visits the child nodes of x.
///      TraverseStmt(Stmt *x) and TraverseType(QualType x) work
///      similarly.
///   2. WalkUpFromFoo(Foo *x) does task #2.  It does not try to visit
///      any child node of x.  Instead, it first calls WalkUpFromBar(x)
///      where Bar is the direct parent class of Foo (unless Foo has
///      no parent), and then calls VisitFoo(x) (see the next list item).
///   3. VisitFoo(Foo *x) does task #3.
///
/// These three method groups are tiered (Traverse* > WalkUpFrom* >
/// Visit*).  A method (e.g. Traverse*) may call methods from the same
/// tier (e.g. other Traverse*) or one tier lower (e.g. WalkUpFrom*).
/// It may not call methods from a higher tier.
///
/// Note that since WalkUpFromFoo() calls WalkUpFromBar() (where Bar
/// is Foo's super class) before calling VisitFoo(), the result is
/// that the Visit*() methods for a given node are called in the
/// top-down order (e.g. for a node of type NamespaceDecl, the order will
/// be VisitDecl(), VisitNamedDecl(), and then VisitNamespaceDecl()).
///
/// This scheme guarantees that all Visit*() calls for the same AST
/// node are grouped together.  In other words, Visit*() methods for
/// different nodes are never interleaved.
///
/// Clients of this visitor should subclass the visitor (providing
/// themselves as the template argument, using the curiously recurring
/// template pattern) and override any of the Traverse*, WalkUpFrom*,
/// and Visit* methods for declarations, types, statements,
/// expressions, or other AST nodes where the visitor should customize
/// behavior.  Most users only need to override Visit*.  Advanced
/// users may override Traverse* and WalkUpFrom* to implement custom
/// traversal strategies.  Returning false from one of these overridden
/// functions will abort the entire traversal.
///
/// By default, this visitor tries to visit every part of the explicit
/// source code exactly once.  The default policy towards templates
/// is to descend into the 'pattern' class or function body, not any
/// explicit or implicit instantiations.  Explicit specializations
/// are still visited, and the patterns of partial specializations
/// are visited separately.  This behavior can be changed by
/// overriding shouldVisitTemplateInstantiations() in the derived class
/// to return true, in which case all known implicit and explicit
/// instantiations will be visited at the same time as the pattern
/// from which they were produced.
///
/// By default, this visitor preorder traverses the AST. If postorder traversal
/// is needed, the \c shouldTraversePostOrder method needs to be overridden
/// to return \c true.
class DeclClosureVisitor : public RecursiveASTVisitor<DeclClosureVisitor>
{
  public:
  DeclClosureVisitor(ClosureSet &closure, EnumConstantTable &table)
    : RecursiveASTVisitor(),
      Closure(closure),
      EnumTable(table)
  {
  }

  /* ---------------- Administrative stuff --------------- */

  /** Tell the RecursiveASTVisitor that we want to visit template instantiations.  */
  bool shouldVisitTemplateInstantiations(void) const //override
  {
    return true;
  }

  /** Check if the Decl was already analyized.  */
  inline bool Already_Analyzed(Decl *decl)
  {
    return AnalyzedDecls.find(decl) != AnalyzedDecls.end();
  }

  inline void Mark_As_Analyzed(Decl *decl)
  {
    AnalyzedDecls.insert(decl);
  }

  /** Check if we can get a partial declaration of `decl` rather than a full
      declaration.  */
  inline TagDecl *Maybe_Partial_Decl(TagDecl *decl)
  {
    if (decl->isCompleteDefinitionRequired()) {
      /* We need the full definition of this struct, enum, or union.  */
      return decl;
    } else {
      /* We don't need the full defintion.  Hide the body for the PrettyPrint
         class.  */
      decl->setCompleteDefinition(false);
      /* FIXME: The PrettyPrint class will attempt to write this declaration
         as the user wrote, that means WITH a body.  To avoid this, we set
         the StartLocation to equal the End of location, which will trigger
         the "clang got confused" mechanism in PrettyPrint and force it to
         be output as a tree dump instead of what the user wrote.  The
         correct way of doing this would be update the source location to
         the correct range.  */
      decl->setLocStart(decl->getEndLoc());
      return decl;
    }
  }

  enum {
    VISITOR_CONTINUE = true,   // Return this for the AST transversal to continue;
    VISITOR_STOP     = false,  // Return this for the AST tranversal to stop completely;
  };

#define TRY_TO(CALL_EXPR)                    \
  if ((CALL_EXPR) == VISITOR_STOP)           \
    return VISITOR_STOP

#define DO_NOT_RUN_IF_ALREADY_ANALYZED(decl) \
  if (Already_Analyzed(decl) == true)        \
    return VISITOR_CONTINUE

  /* Special Traversal functions which marks if a Decl was already analyzed.
     This macro generates them.  */
#define DEF_MARKING_TRAVERSE_DECL(DECL)               \
  bool Traverse##DECL(DECL *decl)                     \
  {                                                   \
    DO_NOT_RUN_IF_ALREADY_ANALYZED(decl);             \
    Mark_As_Analyzed(decl);                           \
    return RecursiveASTVisitor::Traverse##DECL(decl); \
  }

  /* Override of TraverseDecl that marks that the given Decl was analyzed.  So
     far it seems we only need this function for now.  */
  DEF_MARKING_TRAVERSE_DECL(Decl);

  /* -------- C Declarations ----------------- */

  bool VisitFunctionDecl(FunctionDecl *decl)
  {
    if (decl->getBuiltinID() != 0) {
      /* Do not redeclare compiler builtin functions.  */
      return VISITOR_CONTINUE;
    }

    /* If this function is actually a template function specialization, then
       look into it.  */
    if (FunctionTemplateSpecializationInfo *ftsi =
                decl->getTemplateSpecializationInfo()) {
      if (VisitFunctionTemplateDecl(ftsi->getTemplate()) == VISITOR_STOP) {
        return VISITOR_STOP;
      }
    }

    /* Mark function for output.  */
    FunctionDecl *to_mark = decl->isStatic() ? decl->getDefinition() : decl;
    if (to_mark == nullptr) {
      to_mark = decl; //FIXME: Declared static function in closure without a body?
    }
    Closure.Add_Decl_And_Prevs(to_mark);

    return VISITOR_CONTINUE;
  }

  bool VisitValueDecl(ValueDecl *decl)
  {
    TRY_TO(TraverseType(decl->getType()));
    return VISITOR_CONTINUE;
  }

  bool VisitRecordDecl(RecordDecl *decl)
  {
    /* If this is a C++ record decl, then analyze it as such.  */
    if (CXXRecordDecl *cxxdecl = dyn_cast<CXXRecordDecl>(decl)) {
      if (VisitCXXRecordDecl(cxxdecl) == VISITOR_STOP) {
        return VISITOR_STOP;
      }
    }

    if (TypedefNameDecl *typedecl = decl->getTypedefNameForAnonDecl()) {
      /* In case this struct is defined without a name, such as:
        struct {
          int a;
        };

        then do not add it to the output.  This can happen in the case where the
        original code was declared as:

        typedef struct { int a; } A;  */
      return VisitTypedefNameDecl(typedecl);
    } else {
      Closure.Add_Decl_And_Prevs(Maybe_Partial_Decl(decl));
    }

    return VISITOR_CONTINUE;
  }

  bool VisitEnumDecl(EnumDecl *decl)
  {
    if (TypedefNameDecl *typedecl = decl->getTypedefNameForAnonDecl()) {
      /* In case this enum is defined without a name, such as:
        enum {
          CONSTANT = 1,
        };

        then do not add it to the output.  This can happen in the case where the
        original code was declared as:

        typedef enum { CONSTANT = 1 } A;  */
      return VisitTypedefNameDecl(typedecl);
    } else {
      Closure.Add_Decl_And_Prevs(decl);
    }

    return VISITOR_CONTINUE;
  }

  bool VisitTagDecl(TagDecl *decl)
  {
    DeclContext *parent = decl->getParent();
    if (parent && parent->hasValidDeclKind()) {
      /* Check if parent declaration is a namespace.  If yes, then add it
         as well BUT DO NOT add its children.  */
      if (parent->isNamespace()) {
        NamespaceDecl *nm = cast<NamespaceDecl>(parent);
        TRY_TO(VisitNamespaceDecl(nm));
      }
    }

    return VISITOR_CONTINUE;
  }

  bool VisitEnumConstantDecl(EnumConstantDecl *decl)
  {
    /* Add original EnumDecl it originated.  */
    EnumDecl *enum_decl = EnumTable.Get(decl);
    if (enum_decl) {
      return TraverseDecl(enum_decl);
    }

    return VISITOR_CONTINUE;
  }

  /* Not called automatically by Transverse.  */
  bool VisitTypedefNameDecl(TypedefNameDecl *decl)
  {
    TRY_TO(TraverseType(decl->getUnderlyingType()));
    Closure.Add_Decl_And_Prevs(decl);

    return VISITOR_CONTINUE;
  }

  bool VisitRecordType(const RecordType *type)
  {
    return VISITOR_CONTINUE;
  }

  bool VisitVarDecl(VarDecl *decl)
  {
    /* Avoid adding variables that are not global.  */
    if (decl->hasGlobalStorage()) {
      Closure.Add_Decl_And_Prevs(decl);
    }

    return VISITOR_CONTINUE;
  }

  /* --------- C++ Declarations ---------- */

  bool VisitFunctionTemplateDecl(FunctionTemplateDecl *decl)
  {
    Closure.Add_Decl_And_Prevs(decl);

    return VISITOR_CONTINUE;
  }

  bool VisitTemplateDecl(TemplateDecl *decl)
  {
    Closure.Add_Decl_And_Prevs(decl);

    return VISITOR_CONTINUE;
  }

  bool VisitNamespaceDecl(NamespaceDecl *decl)
  {
    Closure.Add_Decl_And_Prevs(decl);

    return VISITOR_CONTINUE;
  }

  bool VisitCXXRecordDecl(CXXRecordDecl *decl)
  {
    /* In case this is a ClassTemplateSpecialDecl, then analyze it as such.  */
    if (ClassTemplateSpecializationDecl *ctsd =
        dyn_cast<ClassTemplateSpecializationDecl>(decl)) {
        if (VisitClassTemplateSpecializationDecl(ctsd) == VISITOR_STOP) {
          return VISITOR_STOP;
        }
    }

    Closure.Add_Decl_And_Prevs(decl);

    return VISITOR_CONTINUE;
  }

  bool VisitClassTemplateDecl(ClassTemplateDecl *decl)
  {
    Closure.Add_Decl_And_Prevs(decl);

    return VISITOR_CONTINUE;
  }

  bool VisitClassTemplateSpecializationDecl(ClassTemplateSpecializationDecl *decl)
  {
    /* Assume
      template<T>
      class V{
        T x;
      };
      struct A {int x; }

      V<A> u;
      This call will add the A to the closure.
    */
    if (TraverseTemplateArguments(decl->getTemplateArgs().asArray()) == VISITOR_STOP) {
      return VISITOR_STOP;
    }

    /* This call will add the V to the closure.  */
    return VisitClassTemplateDecl(decl->getSpecializedTemplate());
  }

  /* ----------- Statements -------------- */

  bool VisitDeclRefExpr(DeclRefExpr *expr)
  {
    TRY_TO(TraverseDecl(expr->getDecl()));
    /* For C++ we also need to analyze the name specifier.  */
    if (NestedNameSpecifier *nns = expr->getQualifier()) {
      TRY_TO(TraverseNestedNameSpecifier(nns));
    }

    /* Analyze the decl it references to.  */
    return TraverseDecl(expr->getDecl());
  }

  bool VisitOffsetOfExpr(OffsetOfExpr *expr)
  {
    /* Handle the following case:
     *
     * typedef struct {
     *   int x;
     *   int y;
     * } Head;
     *
     * int x = __builtin_offsetof(Head, y);
     */
    unsigned num_components = expr->getNumComponents();
    for (unsigned i = 0; i < num_components; i++) {
      /* Get the RecordDecl referenced in the builtin_offsetof and add to the
         dependency list.  */
      const OffsetOfNode &component = expr->getComponent(i);
      if (component.getKind() == OffsetOfNode::Field) {
        const FieldDecl *field = component.getField();

        RecordDecl *record = (RecordDecl *)field->getParent();
        bool ret = TraverseDecl(record);
        if (ret == VISITOR_STOP) {
          return VISITOR_STOP;
        }
      }
    }

    return VISITOR_CONTINUE;
  }

  /* -------- Types ----------------- */
  bool VisitTagType(const TagType *type)
  {
    if (TraverseDecl(type->getDecl()) == VISITOR_STOP) {
      return VISITOR_STOP;
    }
    return VISITOR_CONTINUE;
  }

  bool VisitTypedefType(const TypedefType *type)
  {
    if (TraverseDecl(type->getDecl()) == VISITOR_STOP) {
      return VISITOR_STOP;
    }
    return VISITOR_CONTINUE;
  }

  /* -------- C++ Types ------------- */
  bool VisitTemplateSpecializationType(const TemplateSpecializationType *type)
  {
    /* For some reason the Traverse do not run on the original template
       C++ Record, only on its specializations.  Hence do it here.  */
    return TraverseDecl(type->getAsCXXRecordDecl());
  }

  bool VisitDeducedTemplateSpecializationType(
      const DeducedTemplateSpecializationType *type)
  {
    TemplateName template_name = type->getTemplateName();
    return TraverseDecl(template_name.getAsTemplateDecl());
  }

  bool VisitElaboratedType(const ElaboratedType *type)
  {
    return VISITOR_CONTINUE;
  }

  /* ----------- Other C++ stuff ----------- */
  bool TraverseNestedNameSpecifier(NestedNameSpecifier *nns)
  {
    switch (nns->getKind()) {
      case NestedNameSpecifier::Namespace:
        /* Do not traverse to avoid adding unecessary childs.  */
        TRY_TO(VisitNamespaceDecl(nns->getAsNamespace()));
        break;
      case NestedNameSpecifier::NamespaceAlias:
        TRY_TO(TraverseDecl(nns->getAsNamespaceAlias()));
        break;

      case NestedNameSpecifier::Super:
        /* Something regarding MSVC, but lets put this here.  */
        TRY_TO(TraverseDecl(nns->getAsRecordDecl()));
        break;

      default:
        break;
    }
    return RecursiveASTVisitor::TraverseNestedNameSpecifier(nns);
  }

  bool TraverseCXXBaseSpecifier(const CXXBaseSpecifier base)
  {
    TRY_TO(TraverseType(base.getType()));
    return RecursiveASTVisitor::TraverseCXXBaseSpecifier(base);
  }


  private:
  /** The resulting closure set.  */
  ClosureSet &Closure;

  /** The table maping EnumConstantDecl to its original EnumDecl, used to find
      out where a certain EnumConstantDecl was defined.  */
  EnumConstantTable &EnumTable;

  /** The set of all analyzed Decls.  */
  std::unordered_set<Decl *> AnalyzedDecls;
};

/** FunctionDependencyFinder class methods implementation.  */
FunctionDependencyFinder::FunctionDependencyFinder(PassManager::Context *ctx)
    : AST(ctx->AST.get()),
      EnumTable(AST),
      IT(AST->getPreprocessor(), ctx->HeadersToExpand),
      KeepIncludes(ctx->KeepIncludes),
      Visitor(new DeclClosureVisitor(Closure, EnumTable))
{
}

FunctionDependencyFinder::~FunctionDependencyFinder(void)
{
  delete Visitor;
}

bool FunctionDependencyFinder::Find_Functions_Required(
    std::vector<std::string> const &funcnames)
{
  try {
    for (const std::string &funcname : funcnames) {
      /* Get list of Decls matching the `funcname`.  */
      const DeclContextLookupResult decls = Get_Decl_List_From_Identifier(AST, funcname);

      /* Iterate on them.  */
      for (Decl *decl : decls) {
        Visitor->TraverseDecl(decl);
      }
    }
  } catch (SymbolNotFoundException &e) {
    DiagsClass::Emit_Error(std::string("Requested symbol not found: ") + e.what());
    return false;
  }

  return true;
}

void FunctionDependencyFinder::Print(void)
{
  RecursivePrint(AST, Closure.Get_Set(), IT, KeepIncludes).Print();
}

void FunctionDependencyFinder::Remove_Redundant_Decls(void) {
  std::unordered_set<Decl *> &closure_set = Closure.Get_Set();

  for (auto it = closure_set.begin(); it != closure_set.end(); ++it) {
    /* Handle the case where a enum or struct is declared as:

        typedef enum Hand {
          LEFT,
          RIGHT
        } Hand;

        which is then broken as

        enum Hand {
          LEFT,
          RIGHT
        };

        typedef enum Hand Hand;

        but when printing, we track the typedef declaration to what the user
        wrote and it will be transformed into

        enum Hand {
          LEFT,
          RIGHT
        };

        typedef enum Hand {
          LEFT,
          RIGHT
        } Hand;

        which is a redeclaration of enum Hand. Hence we have to remove the
        first `enum Hand` from the closure.  See typedef-7.c testcase.  */
    if (TypedefDecl *decl = dynamic_cast<TypedefDecl *>(*it)) {
      SourceRange range = decl->getSourceRange();

      const Type *type = decl->getTypeForDecl();
      if (type) {
        TagDecl *typedecl = type->getAsTagDecl();

        if (typedecl && Closure.Is_Decl_Marked(typedecl)) {
          SourceRange type_range = typedecl->getSourceRange();

          /* Check if the strings regarding an decl empty. In that case we
             can not delete the decl from list.  */
          if (PrettyPrint::Get_Source_Text(range) != "" &&
              PrettyPrint::Get_Source_Text(type_range) != "") {

            /* Using .fullyContains() fails in some declarations.  */
            if (PrettyPrint::Contains_From_LineCol(range, type_range)) {
              Closure.Remove_Decl(typedecl);
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
      if (typedecl && Closure.Is_Decl_Marked(typedecl)) {
        SourceRange type_range = typedecl->getSourceRange();

        /* Using .fullyContains() fails in some declarations.  */
        if (PrettyPrint::Contains_From_LineCol(range, type_range)) {
          Closure.Remove_Decl(typedecl);
        }
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
      Visitor->TraverseDecl(decl);
    }
  }
}

bool FunctionDependencyFinder::Run_Analysis(std::vector<std::string> const &functions)
{
  bool ret = true;
  /* Step 1: Compute the closure.  */
  ret &= Find_Functions_Required(functions);

  /* Step 2: Expand the closure to include Decls in non-expanded includes.  */
  if (KeepIncludes) {
    /* TODO: Guarding this with KeepIncludes should actually not be necessary,
       but we still have bugs when processing the kernel and the closure is not
       perfect.  By not adding those declarations we avoid hitting cases where
       it would hang because of a declaration that would be removed in the
       future anyway.  It also speeds up analysis considerably when the user do
       not want to keep the includes.  */
    Insert_Decls_From_Non_Expanded_Includes();
  }

  /* Step 3: Remove any declaration that may have been declared twice.  This
     is required because of the `enum-5.c` testcase (struct declared inside
     a typedef).  */
  Remove_Redundant_Decls();

  return ret;
}
