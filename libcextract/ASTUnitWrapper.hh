//===- ASTUnitWrapper.hh - Wrapper around clang's ASTUnit ------------*- C++ *-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Wrapper around clang's ASTUnit.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#pragma once

#include <clang/Frontend/ASTUnit.h>
#include <llvm/Support/VirtualFileSystem.h>

#include <memory>
#include <vector>

using namespace clang;

/** @brief Wrapper around clang's ASTUnit.
 *
 * This class owns clang's ASTUnit and is the place where clang-extract
 * extends it, as ASTUnit can not be subclassed: its constructor is private and
 * it is only created through static factories.  It also hides the hacks
 * required to create an ASTUnit with a virtual filesystem.
 */
class ASTUnitWrapper
{
  public:
  /** Parse the source file given in the clang command line `args`, accessing
      files through the filesystem `fs`.  Returns nullptr in case the command
      line is invalid, e.g. no input file was given.  */
  static std::unique_ptr<ASTUnitWrapper>
  Create(const std::vector<const char *> &args,
         IntrusiveRefCntPtr<llvm::vfs::FileSystem> fs,
         bool ignore_clang_errors);

  /** Parse the source file again, accessing files through `fs`.  Returns true
      on success.  */
  bool Reparse(IntrusiveRefCntPtr<llvm::vfs::FileSystem> fs);

  /** Gets the portion of the code that corresponds to given SourceRange, including the
      last token. Returns expanded macros.  */
  StringRef Get_Source_Text(const SourceRange &range);

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
  StringRef Get_Source_Text_Raw(const SourceRange &range);

  /** Get the TopLevel Decl that contains the location loc.  */
  Decl *Get_Toplevel_Decl_At_Location(const SourceLocation &loc);

  /** Get Toplevel decls with same beginloc.  */
  ArrayRef<Decl *> Get_Toplev_Decls_With_Same_Beginloc(const SourceLocation &loc);

  /** Lookup in the symbol table for a declaration with given name passed by info.  */
  DeclContextLookupResult Get_Decl_From_Symtab(const IdentifierInfo *info);

  /** Lookup in the symbol table for a declaration with given name passed by name.  */
  DeclContextLookupResult Get_Decl_From_Symtab(const StringRef &name);

  /* ---- Forwarders to the ASTUnit ---- */

  inline SourceManager &getSourceManager(void)
  { return AST->getSourceManager(); }

  inline Preprocessor &getPreprocessor(void)
  { return AST->getPreprocessor(); }

  inline ASTContext &getASTContext(void)
  { return AST->getASTContext(); }

  inline const LangOptions &getLangOpts(void)
  { return AST->getLangOpts(); }

  inline DiagnosticsEngine &getDiagnostics(void)
  { return AST->getDiagnostics(); }

  inline ASTUnit::top_level_iterator top_level_begin(void)
  { return AST->top_level_begin(); }

  inline ASTUnit::top_level_iterator top_level_end(void)
  { return AST->top_level_end(); }

  inline bool isInMainFileID(SourceLocation loc)
  { return AST->isInMainFileID(loc); }

  /** Get the underlying ASTUnit, for code that has not been ported to this
      class yet or clang APIs that require it.  */
  inline ASTUnit *Get_ASTUnit(void)
  { return AST.get(); }

  /** Get the last created ASTUnitWrapper object which is still alive, or
      nullptr.  Used by the functions designed to be called from gdb, which
      have no other way of reaching it.  */
  static inline ASTUnitWrapper *Get_Last_Created(void)
  { return Last_Created; }

  ~ASTUnitWrapper(void)
  {
    if (Last_Created == this) {
      Last_Created = nullptr;
    }
  }

  private:
  ASTUnitWrapper(std::unique_ptr<ASTUnit> ast)
    : AST(std::move(ast))
  {
    Last_Created = this;
  }

  /** The wrapped ASTUnit object.  */
  std::unique_ptr<ASTUnit> AST;

  /** The last created ASTUnitWrapper object which is still alive.  */
  static ASTUnitWrapper *Last_Created;
};
