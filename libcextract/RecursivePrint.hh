//===- RecursivePrint.hh - Print Decls or Preprocessor entities to file *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Print Decls or Preprocessor entities to file.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#pragma once

#include <unordered_set>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/raw_ostream.h>

#include "IncludeTree.hh"
#include "MacroWalker.hh"

using namespace clang;

/** @brief Print the decls marked for output together with the macros.
 *
 * Starting from the translation unit, iterate through the decls of each
 * DeclContext and the preprocessed entities inside of it in the order they
 * appear in the source code, and print the ones that are needed.  Namespaces
 * and `extern "C"` blocks are printed recursively, so only what is needed in
 * them are output.
 */
class RecursivePrint
{
  public:
  RecursivePrint(ASTUnit *ast,
                 std::unordered_set<Decl *> &deps,
                 IncludeTree &it,
                 raw_ostream &out,
                 bool keep_includes,
                 bool no_duplicated_includes = false);

  /* Print output to `Out`.  */
  void Print(void);

  /** Print the decls and preprocessed entities inside of a DeclContext.  */
  void Print_Decl_Context(DeclContext *ctx);

  /** Print single decl.  In case it is a namespace or an `extern "C"` block,
      print what is needed inside of it.  */
  void Print_Decl(Decl *decl);

  void Print_Preprocessed(PreprocessedEntity *prep);

  /** Print a Macro Undef into ostream `Out`.  */
  void Print_Macro_Undef(MacroDirective *directive);

  protected:

  /** Print a namespace and what is needed inside of it.  */
  void Print_Namespace(NamespaceDecl *decl);

  /** Print an `extern "C"` block and what is needed inside of it.  */
  void Print_Linkage_Spec(LinkageSpecDecl *decl);

  /** Print Decl as written in the source code, or dump it from the AST in
      case it is not possible.  */
  void Print_Decl_Raw(Decl *decl);

  /** Print the macro defined in MacroInfo.  */
  void Print_MacroInfo(MacroInfo *info);

  /** Check if a namespace or `extern "C"` block is marked for output or have
      anything marked for output inside of it.  */
  bool Is_Decl_Context_Needed(Decl *decl);

  /** Get the location of the end of decl, including its attributes.  */
  SourceLocation Get_Expanded_Loc(Decl *decl);

  /** Check if range is valid and not empty.  */
  bool Is_Range_Valid(const SourceRange &range);

  /** Get the source code in range.  */
  StringRef Get_Source_Text(const SourceRange &range);

  /** Print a clang-extract comment that is on top of each declaration.  */
  void Print_CE_Comment(Decl *decl);

  /** Print a clang-extract comment that is on top of each macro.  */
  void Print_CE_Comment(MacroInfo *minfo);

  /** Print a clang-extract comment on location.  */
  void Print_CE_Comment(const SourceLocation &loc, RawComment *comment);

  /** Remove decls and macros that are already covered by includes that are
      marked for output.  */
  void Analyze_Includes(bool no_duplicated_includes);

  /** Check if a given declaration was already marked as dependency.  */
  inline bool Is_Decl_Marked(Decl *decl)
  { return Decl_Deps.find(decl) != Decl_Deps.end(); }

  /** Determine if a macro that are marked for output.  */
  inline bool Is_Macro_Marked(MacroInfo *x)
  { return x && x->isUsed(); }

  inline void Unmark_Decl(Decl *decl)
  { Decl_Deps.erase(decl); }

  inline void Unmark_Macro(MacroInfo *x)
  { x->setIsUsed(false); }

  ASTUnit *AST;
  SourceManager &SM;
  const LangOptions &LangOpts;
  PrintingPolicy PPolicy;
  MacroWalker MW;
  std::unordered_set<Decl *> &Decl_Deps;
  IncludeTree &IT;

  /** Where the output is written to.  */
  raw_ostream &Out;

  bool KeepIncludes;
};
