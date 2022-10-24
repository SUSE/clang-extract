#pragma once

#include "MacroWalker.hh"

#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>

using namespace clang;

/** Class encapsulating the Symbol externalizer mechanism.
 *
 * Livepatched functions very often references static functions or variables
 * that we have to `externalize` them.  Assume we want to externalize a
 * function f. We externalize it by applying the following transformation:
 *
 * before:
 * int f(void) {
 *  ... code
 *  return x;
 * }
 *
 * after:
 * static int (*f)(void);
 *
 * then libpulp or klp is responsible for initializing the pointer `f` with
 * its correct address in the target binary.
 *
 */
class SymbolExternalizer
{
  public:
  SymbolExternalizer(ASTUnit *ast)
    : AST(ast),
      MW(ast->getPreprocessor()),
      RW(ast->getSourceManager(), ast->getLangOpts())
  {
  }

  class FunctionUpdater
  {
    public:
    /** A reference to FunctionUpdater's Rewriter instance.  */
    Rewriter &RW;

    /** The new variable declaration to replace the to be externalized function.  */
    VarDecl *NewSymbolDecl;

    /** Name of the to be replaced function.  */
    const std::string &OldSymbolName;

    FunctionUpdater(Rewriter &rw, VarDecl *new_decl, const std::string &old_decl_name, bool was_function)
    : RW(rw), NewSymbolDecl(new_decl), OldSymbolName(old_decl_name), WasFunction(was_function)
    {}

    /** Sweeps the function and update any reference to the old function, replacing
        it with the externalized variable.  */
    bool Update_References_To_Symbol(DeclaratorDecl *to_update);

    private:

    /* Decl to update.  */
    DeclaratorDecl *ToUpdate;

    /* Was the original declaration a function?  */
    bool WasFunction;

    bool Update_References_To_Symbol(Stmt *);
  };
  friend class FunctionUpdater;

  /* Create the externalized var as a AST node ready to be slapped into the
     AST.  */
  VarDecl *Create_Externalized_Var(DeclaratorDecl *decl, const std::string &name);

  /** Externalize a symbol, that means transforming functions into a function
      pointer, or an global variable into a variable pointer.  */
  void Externalize_Symbol(DeclaratorDecl *to_externalize);
  void Externalize_Symbol(const std::string &to_externalize);
  void Externalize_Symbols(std::vector<std::string> const &to_externalize_array);

  /** Commit changes to the loaded source file buffer.  Should NOT modify the
      original file, only the content that was loaded in llvm's InMemory file
      system.  */
  bool Commit_Changes_To_Source(void);

  std::string Get_Modifications_To_Main_File(void);

  private:

  void _Externalize_Symbol(const std::string &to_externalize);

  void Rewrite_Macros(std::string const &to_look_for, std::string const &replace_with);

  /** MacroWalker object which helps iterating on Macros.  */
  MacroWalker MW;

  /** AST in analysis.  */
  ASTUnit *AST;

  /** Clang's Rewriter class used to auxiliate us with changes in the source code.  */
  Rewriter RW;
};
