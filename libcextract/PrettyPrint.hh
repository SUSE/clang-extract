//===- PrettyPrint.hh - Print Decls or Preprocessor entities to file *- C++ -*-===//
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
#include "TopLevelASTIterator.hh"

using namespace clang;

/** @brief Print the Decls and macros marked for output.
 *
 * Clang AST pretty printer does a good job at printing declaration nodes. Just
 * calling decl->print(params) will also print its children. However there are
 * some caveats regarding some nodes, which are handled here.  Whenever
 * possible the code is printed as written in the source code rather than
 * dumped from the AST.
 */
class RecursivePrint
{
  public:
  RecursivePrint(ASTUnitWrapper &ast,
                 std::unordered_set<Decl *> &deps,
                 raw_ostream &out,
                 IncludeTree &it,
                 bool keep_includes,
                 bool no_duplicated_includes = false);

  /** Same as above, but the IncludeTree is optional.  It can be nullptr in
      case we don't keep the includes, as it is only used then.  */
  RecursivePrint(ASTUnitWrapper &ast,
                 std::unordered_set<Decl *> &deps,
                 raw_ostream &out,
                 IncludeTree *it = nullptr,
                 bool keep_includes = false,
                 bool no_duplicated_includes = false);

  /* Print output to `Out`.  */
  void Print(void);

  /** Print single decl.  */
  void Print_Decl(Decl *decl);

  /** Print a Macro Defintion into ostream `Out`.  */
  void Print_Macro(MacroDefinitionRecord *rec);

  void Print_Preprocessed(PreprocessedEntity *prep);

  /** Print a Macro Undef into ostream `Out`.  */
  void Print_Macro_Undef(MacroDirective *directive);

  protected:

  /** Print Decl node as is, without any kind of processing.  */
  void Print_Decl_Raw(Decl *decl);

  /** Print the macro defined in info.  */
  void Print_MacroInfo(MacroInfo *info);

  /** Get the furthest location of decl, including its attributes.  */
  SourceLocation Get_Expanded_Loc(Decl *decl);

  /** Check if range is valid and not empty.  */
  bool Is_Range_Valid(const SourceRange &range);

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

  ASTUnitWrapper &AST;
  TopLevelASTIterator ASTIterator;
  MacroWalker MW;
  std::unordered_set<Decl *> &Decl_Deps;
  /** The tree of includes.  Can be nullptr if we don't keep the includes.  */
  IncludeTree *IT;

  /** Output object to where this class will output to.  */
  raw_ostream &Out;

  /** Language options used by clang's internal PrettyPrinter.  We use the
      default options for now.  */
  LangOptions LangOpts;

  /** Policy for printing.  We use the default for now.  */
  PrintingPolicy PPolicy;

  bool KeepIncludes;

  /* Vector of MacroDirective of macros that needs to be undefined somewhere in
     the code.  */
  std::vector<MacroDirective*> NeedsUndef;
};

/* Functions designed to be called from gdb.  They take a void pointer because
   gdb usually don't have the type information of clang classes, hence it can't
   convert e.g. a `DeclRefExpr *` into a `Stmt *` when calling them.  Since
   `Decl` and `Stmt` are always the leftmost base class, any pointer to a class
   derived from them can be passed.  */
extern "C" {

/** Print the Decl pointed by decl to stderr the same way it would be printed
    in the output, e.g. `call Debug_Decl(decl)`.  The decl must be in the last
    created ASTUnitWrapper.  */
void Debug_Decl(void *decl);

/** Print the Stmt pointed by stmt to stderr, e.g. `call Debug_Stmt(expr)`.  */
void Debug_Stmt(void *stmt);

}
