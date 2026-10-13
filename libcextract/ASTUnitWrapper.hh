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
 * required to create an ASTUnit with a virtual filesystem (see
 * ASTUnitHack.cpp).
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

  private:
  ASTUnitWrapper(std::unique_ptr<ASTUnit> ast)
    : AST(std::move(ast))
  {
  }

  /** The wrapped ASTUnit object.  */
  std::unique_ptr<ASTUnit> AST;
};
