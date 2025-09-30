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
#include <clang/Basic/Version.h>

#if CLANG_VERSION_MAJOR >= 21
#include <clang/Serialization/ModuleCache.h>
#endif

using namespace clang;

/** Common code for the ASTUnit::create functions.  */
#define ASTUNIT_COMMON(CI, Diags, CaptureDiagnostics, UserFilesAreVolatile) \
  std::unique_ptr<ASTUnit> AST(new ASTUnit(false));                         \
  ConfigureDiags(Diags, *AST, CaptureDiagnostics);                          \
  AST->Diagnostics = Diags;                                                 \
  AST->FileSystemOpts = CI->getFileSystemOpts();                            \
  AST->Invocation = std::move(CI);                                          \
  AST->FileMgr = new FileManager(AST->FileSystemOpts, _Hack_VFS);           \
  AST->UserFilesAreVolatile = UserFilesAreVolatile;                         \
  AST->SourceMgr = new SourceManager(AST->getDiagnostics(), *AST->FileMgr,  \
                                     UserFilesAreVolatile)

/** Filesystem that will be used in our custom ASTUnit::create, so that way we
  * don't break clang's API.
  */
IntrusiveRefCntPtr<llvm::vfs::FileSystem> _Hack_VFS;


/** Hack to create an ASTUnit from LoadFromCompilerInvocationAction with a
 *  virtual filesystem.  That way we can use FrontendActions hooks when
 *  creating the ASTUnit.
 */
#if CLANG_VERSION_MAJOR >= 21
std::unique_ptr<ASTUnit>
ASTUnit::create(std::shared_ptr<CompilerInvocation> CI,
                std::shared_ptr<DiagnosticOptions> DiagOpts,
                IntrusiveRefCntPtr<DiagnosticsEngine> Diags,
                CaptureDiagsKind CaptureDiagnostics,
                bool UserFilesAreVolatile) {

  ASTUNIT_COMMON(CI, Diags, CaptureDiagnostics, UserFilesAreVolatile);
  AST->DiagOpts = DiagOpts;
  AST->ModCache = createCrossProcessModuleCache();
  return AST;
}
#else
std::unique_ptr<ASTUnit>
ASTUnit::create(std::shared_ptr<CompilerInvocation> CI,
                IntrusiveRefCntPtr<DiagnosticsEngine> Diags,
                CaptureDiagsKind CaptureDiagnostics,
                bool UserFilesAreVolatile) {
  ASTUNIT_COMMON(CI, Diags, CaptureDiagnostics, UserFilesAreVolatile);
  AST->ModuleCache = new InMemoryModuleCache;
  return AST;
}
#endif
