#pragma once

#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>

using namespace clang;

/** Class encapsulating the function externalizer mechanism.
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
class FunctionExternalizer
{
  public:
  FunctionExternalizer(ASTUnit *ast)
    : AST(ast),
      RW(AST->getSourceManager(), AST->getLangOpts())
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

    FunctionUpdater(Rewriter &rw, VarDecl *new_decl, const std::string &old_decl_name)
    : RW(rw), NewSymbolDecl(new_decl), OldSymbolName(old_decl_name)
    {}

    /** Sweeps the function and update any reference to the old function, replacing
        it with the externalized variable.  */
    bool Update_References_To_Symbol(FunctionDecl *to_update);

    private:

    /* Decl to update.  */
    FunctionDecl *ToUpdate;

    bool Update_References_To_Symbol(Stmt *);
  };
  friend class FunctionUpdater;

  /* Create the externalized var as a AST node ready to be slapped into the
     AST.  */
  VarDecl *Create_Externalized_Var(FunctionDecl *decl, const std::string &name);

  /** Externalize a symbol, that means transforming functions into a function
      pointer, or an global variable into a variable pointer.  */
  void Externalize_Symbol(DeclaratorDecl *to_externalize);
  void Externalize_Symbol(const std::string &to_externalize);

  /** Commit changes to the loaded source file buffer.  Should NOT modify the
      original file, only the content that was loaded in llvm's InMemory file
      system.  */
  bool Commit_Changes_To_Source(void);

  private:

  /** AST in analysis.  */
  ASTUnit *AST;

  /** Clang's Rewriter class used to auxiliate us with changes in the source code.  */
  Rewriter RW;
};
