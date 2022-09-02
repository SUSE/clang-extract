#pragma once

#include <clang/Tooling/Tooling.h>

using namespace clang;

/** @brief Wrapper class to printing clang AST nodes.
 *
 * Clang AST pretty printer does a good job at printing declaration nodes. Just
 * calling decl->print(params) will also print its children. However there are
 * some caveats regarding some nodes, in which the methods `Print_Decl` and
 * `Print_Stmt` are used to detect and print them correctly.
 */
class PrettyPrint
{
  public:
  /** Print a Decl node into ostream `Out`.  */
  static void Print_Decl(Decl *decl);

  /** Print a Stmt node into ostream `Out`.  */
  static void Print_Stmt(Stmt *stmt);

  /* This class can not be initialized.  */
  PrettyPrint() = delete;

  private:
  /** Output object to where this class will output to.  Current default is the
      same as llvm::outs().  */
  static raw_ostream &Out;

  /** Language options used by clang's internal PrettyPrinter.  We use the
      default options for now.  */
  static LangOptions LangOpts;

  /** Policy for printing.  We use the default for now.  */
  static PrintingPolicy PPolicy;
};

/** Since writing PrettyPrint::Print_Decl can be bothering and result in
    unecessary long code, we also provide those non-namespaced functions.  */
static inline void Print_Decl(Decl *decl)
{
  PrettyPrint::Print_Decl(decl);
}

static inline void Print_Stmt(Stmt *stmt)
{
  PrettyPrint::Print_Stmt(stmt);
}
