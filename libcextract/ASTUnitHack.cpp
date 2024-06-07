//===- ASTUnitHack.cpp - Hacks for the ASTUnit class ------------------*- C++ *-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Hacks for the ASTUnit class.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Serialization/InMemoryModuleCache.h>

using namespace clang;

/** Filesystem that will be used in our custom ASTUnit::create, so that way we
  * don't break clang's API.
  */
IntrusiveRefCntPtr<llvm::vfs::FileSystem> _Hack_VFS;

/** Hack to create an ASTUnit from LoadFromCompilerInvocationAction with a
 *  virtual filesystem.  That way we can use FrontendActions hooks when
 *  creating the ASTUnit.
 */
std::unique_ptr<ASTUnit>
ASTUnit::create(std::shared_ptr<CompilerInvocation> CI,
                IntrusiveRefCntPtr<DiagnosticsEngine> Diags,
                CaptureDiagsKind CaptureDiagnostics,
                bool UserFilesAreVolatile) {

  std::unique_ptr<ASTUnit> AST(new ASTUnit(false));
  ConfigureDiags(Diags, *AST, CaptureDiagnostics);

  AST->Diagnostics = Diags;
  AST->FileSystemOpts = CI->getFileSystemOpts();
  AST->Invocation = std::move(CI);
  AST->FileMgr = new FileManager(AST->FileSystemOpts, _Hack_VFS);
  AST->UserFilesAreVolatile = UserFilesAreVolatile;
  AST->SourceMgr = new SourceManager(AST->getDiagnostics(), *AST->FileMgr,
                                     UserFilesAreVolatile);
  AST->ModuleCache = new InMemoryModuleCache;

  return AST;
}
