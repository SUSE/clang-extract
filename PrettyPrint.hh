#pragma once

#include <clang/Tooling/Tooling.h>
#include "llvm/Support/raw_ostream.h"

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

  /** Print Decl node as is, without any kind of processing.  */
  static void Print_Decl_Raw(Decl *decl);

  static inline void Print_Decl_Tree(Decl *decl)
  { decl->print(*Out, PPolicy); *Out << '\n'; }

  static inline void Debug_Decl_Tree(Decl *decl)
  { decl->print(llvm::outs(), PPolicy); llvm::outs() << '\n'; }

  static void Debug_Decl(Decl *decl);
  static void Debug_Stmt(Stmt *stmt);

  /** Print a Stmt node into ostream `Out`.  */
  static void Print_Stmt(Stmt *stmt);

  /** Print a Macro Defintion into ostream `Out`.  */
  static void Print_Macro_Def(MacroDefinitionRecord *rec);

  /** Print a Macro Undef into ostream `Out`.  */
  static void Print_Macro_Undef(MacroDirective *directive);

  static void Print_MacroInfo(MacroInfo *info);

  static void Print_Attr(Attr *attr);

  static bool Contains_From_LineCol(const SourceRange &a, const SourceRange &b);

  static bool Contains(const SourceRange &a, const SourceRange &b);

  static inline void Set_Source_Manager(SourceManager *sm)
  {
    SM = sm;
  }

  static inline SourceManager *Get_Source_Manager(void)
  {
    return SM;
  }

  static inline LangOptions &Get_Lang_Options(void)
  {
    return LangOpts;
  }

  static void Set_Output_Ostream(llvm::raw_ostream *out)
  {
    Out = out;
  }

  /** Gets the portion of the code that corresponds to given SourceRange, including the
      last token. Returns expanded macros.

      @see get_source_text_raw().  */
  static StringRef Get_Source_Text(const SourceRange &range);

  /** Gets the portion of the code that corresponds to given SourceRange exactly as
      the range is given.

      @warning The end location of the SourceRange returned by some Clang functions
      (such as clang::Expr::getSourceRange) might actually point to the first character
      (the "location") of the last token of the expression, rather than the character
      past-the-end of the expression like clang::Lexer::getSourceText expects.
      get_source_text_raw() does not take this into account. Use get_source_text()
      instead if you want to get the source text including the last token.

      @warning This function does not obtain the source of a macro/preprocessor expansion.
      Use get_source_text() for that.   */
  static StringRef Get_Source_Text_Raw(const SourceRange &range);

  /** Check if SourceLocation a is located before than b in the SourceCode.  */
  static bool Is_Before(const SourceLocation &a, const SourceLocation &b);

  /** Check if SourceLocation a is located after than b in the SourceCode.  */
  static bool Is_After(const SourceLocation &a, const SourceLocation &b);

  /** Set output to file.  */
  static void Set_Output_To(const std::string &path);

  static SourceLocation Get_Expanded_Loc(Decl *decl);

  /* This class can not be initialized.  */
  PrettyPrint() = delete;

  private:

  static bool Is_Range_Valid(const SourceRange &loc);

  /** Output object to where this class will output to.  Current default is the
      same as llvm::outs().  */
  static raw_ostream *Out;

  /** Language options used by clang's internal PrettyPrinter.  We use the
      default options for now.  */
  static LangOptions LangOpts;

  /** Policy for printing.  We use the default for now.  */
  static PrintingPolicy PPolicy;

  /** SourceManager built when parsing the AST.  Must be set after constructing
      the ast by calling Set_Source_Manager.  */
  static SourceManager *SM;
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

static inline bool Is_Loc_Before(const SourceLocation &a, const SourceLocation &b)
{
  return PrettyPrint::Is_Before(a, b);
}
