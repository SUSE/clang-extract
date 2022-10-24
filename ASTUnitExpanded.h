#pragma once

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>

using namespace clang;

class ASTUnitExpanded
{
  public:
  ASTUnitExpanded(std::unique_ptr<ASTUnit> ast)
    : AST(std::move(ast)),
      PPolicy(AST->getLangOpts()),
      Out(llvm::outs())
  {}


  /** Methods imported from ASTUnit.  */
  llvm::StringMap<unsigned> &getCachedCompletionTypes();

   /// Retrieve the allocator used to cache global code completions.
  std::shared_ptr<GlobalCodeCompletionAllocator> getCachedCompletionAllocator();

  CodeCompletionTUInfo &getCodeCompletionTUInfo();

  /** Methods expanded.  */

  void Print_Decl(Decl *decl);

  /** Print Decl node as is, without any kind of processing.  */
  void Print_Decl_Raw(Decl *decl);

  inline void Print_Decl_Tree(Decl *decl)
  { decl->print(Out, PPolicy); Out << '\n'; }

  /** Print a Stmt node into ostream `Out`.  */
  void Print_Stmt(Stmt *stmt);

  /** Print a Macro Defintion into ostream `Out`.  */
  void Print_Macro_Def(MacroDefinitionRecord *rec);

  /** Print a Macro Undef into ostream `Out`.  */
  void Print_Macro_Undef(MacroDirective *directive);

  void Print_MacroInfo(MacroInfo *info);

  bool Contains(const SourceRange &a, const SourceRange &b);

  bool Is_Range_Valid(const SourceRange &loc);


  private:
  std::unique_ptr<ASTUnit> AST;
  PrintingPolicy PPolicy;
  raw_ostream &Out;
};
