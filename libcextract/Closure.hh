//===- Closure.hh - Compute dependencies of a given symbol *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Compute a set of Decls that is necessary to compile a given symbol.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#pragma once

#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Sema/IdentifierResolver.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <unordered_set>

#include "LLVMMisc.hh"
#include "PrettyPrint.hh"

using namespace clang;

class ClosureSet
{
  public:
  /** Check if a given declaration was already marked as dependency.  */
  inline bool Is_Decl_Marked(Decl *decl)
  { return Dependencies.find(decl) != Dependencies.end(); }

  /** Mark decl as dependencies and all its previous decls versions.  */
  bool Add_Decl_And_Prevs(Decl *decl);

  /** Add a single decl to the set.  */
  bool Add_Single_Decl(Decl *decl)
  {
    /* Do not insert builtin decls.  */
    if (Is_Builtin_Decl(decl)) {
      return false;
    }

    Dependencies.insert(decl);
    return true;
  }

  inline std::unordered_set<Decl *> &Get_Set(void)
  {
    return Dependencies;
  }

  inline void Remove_Decl(Decl *decl)
  {
    Dependencies.erase(decl);
  }

  private:
  /** Datastructure holding all Decls required for the functions. This is
      then used to mark which Decls we need to output.

      We use a unordered_set because we need quick lookup, so a hash table
      may be (?) the ideal datastructure for this.  */
  std::unordered_set<Decl*> Dependencies;
};


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
  DeclClosureVisitor(ASTUnit *ast)
    : RecursiveASTVisitor(),
      AST(ast)
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

  enum {
    VISITOR_CONTINUE = true,   // Return this for the AST transversal to continue;
    VISITOR_STOP     = false,  // Return this for the AST tranversal to stop completely;
  };


  /* Special Traversal functions which marks if a Decl was already analyzed.
     Override of TraverseDecl that marks that the given Decl was analyzed.  So
     far it seems we only need this function for now.  */
  bool TraverseDecl(Decl *decl);

  /* -------- C Declarations ----------------- */

  bool VisitFunctionDecl(FunctionDecl *decl);

  bool VisitDeclaratorDecl(DeclaratorDecl *decl);

  bool VisitValueDecl(ValueDecl *decl);

  bool VisitRecordDecl(RecordDecl *decl);

  bool VisitEnumDecl(EnumDecl *decl);

  bool VisitTagDecl(TagDecl *decl);

  bool VisitEnumConstantDecl(EnumConstantDecl *decl);

  /* Not called automatically by Transverse.  */
  bool VisitTypedefNameDecl(TypedefNameDecl *decl);

  bool VisitVarDecl(VarDecl *decl);

  /* --------- C++ Declarations ---------- */

  bool VisitFunctionTemplateDecl(FunctionTemplateDecl *decl);

  bool VisitTemplateDecl(TemplateDecl *decl);

  bool VisitNamespaceDecl(NamespaceDecl *decl);

  bool VisitCXXRecordDecl(CXXRecordDecl *decl);

  bool VisitClassTemplateDecl(ClassTemplateDecl *decl);

  bool VisitClassTemplateSpecializationDecl(ClassTemplateSpecializationDecl *decl);

  /* ----------- Statements -------------- */

  bool VisitDeclRefExpr(DeclRefExpr *expr);

  bool VisitOffsetOfExpr(OffsetOfExpr *expr);

  bool VisitConvertVectorExpr(const ConvertVectorExpr *expr);

  bool VisitUnaryOperator(const UnaryOperator *expr);

  /* --------- Attributes ----------- */
  bool VisitCleanupAttr(const CleanupAttr *attr);

  /* -------- Types ----------------- */
  bool VisitTagType(const TagType *type);

  bool VisitTypedefType(const TypedefType *type);

  /* -------- C++ Types ------------- */
  bool VisitTemplateSpecializationType(const TemplateSpecializationType *type);

  bool VisitDeducedTemplateSpecializationType(
      const DeducedTemplateSpecializationType *type);

  /* ----------- Other C++ stuff ----------- */
  bool TraverseNestedNameSpecifier(NestedNameSpecifier *nns);

  bool TraverseCXXBaseSpecifier(const CXXBaseSpecifier base);

  /* ------ Helper Functions ------ */

  bool ParentRecordDeclHelper(TagDecl *decl);

  bool AnalyzeDeclsWithSameBeginlocHelper(Decl *decl);

  bool AnalyzePreviousDecls(Decl *decl);

  ClosureSet &Get_Closure(void)
  {
    return Closure;
  }

  const ClosureSet &Get_Closure(void) const
  {
    return Closure;
  }

  void Compute_Closure_Of_Symbols(const std::vector<std::string> &names,
                                 std::unordered_set<std::string> *matched_names = nullptr);

  private:

  /** The ASTUnit object.  */
  ASTUnit *AST;

  /** Datastructure holding all Decls required for the functions. This is
      then used to mark which Decls we need to output.

      We use a unordered_set because we need quick lookup, so a hash table
      may be (?) the ideal datastructure for this.  */
  ClosureSet Closure;

  /** The set of all analyzed Decls.  */
  std::unordered_set<Decl *> AnalyzedDecls;
};
