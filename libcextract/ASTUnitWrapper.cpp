//===- ASTUnitWrapper.cpp - Wrapper around clang's ASTUnit -----------*- C++ *-===//
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

#include "ASTUnitWrapper.hh"
#include "ClangCompat.hh"

#include <stdexcept>

#include <clang/Serialization/InMemoryModuleCache.h>

#if CLANG_VERSION_MAJOR >= 21
#include <clang/Serialization/ModuleCache.h>
#endif

/** Filesystem that will be used in our custom ASTUnit::create, so that way we
  * don't break clang's API.
  */
IntrusiveRefCntPtr<llvm::vfs::FileSystem> _Hack_VFS;

std::unique_ptr<ASTUnitWrapper>
ASTUnitWrapper::Create(const std::vector<const char *> &args,
                        IntrusiveRefCntPtr<llvm::vfs::FileSystem> fs,
                        bool ignore_clang_errors)
{
  _Hack_VFS = fs;

  /* Built the ASTUnit from the passed command line.  */
  auto diagopts = ClangCompat::createDiagnosticOptionsWithColor();

  IntrusiveRefCntPtr<DiagnosticsEngine> Diags =
    ClangCompat::createDiagnostics(*_Hack_VFS, diagopts);

  if (ignore_clang_errors) {
    Diags->setWarningsAsErrors(false);
    Diags->setErrorsAsFatal(false);
    Diags->setIgnoreAllWarnings(true);
  }

  /* Create invocation object.  */
  clang::CreateInvocationOptions CIOpts;
  CIOpts.Diags = Diags;
  std::shared_ptr<CompilerInvocation> CInvok =
    clang::createInvocation(args, std::move(CIOpts));

  /* Try no not crash if no input file is given.  */
  if (CInvok == nullptr) {
    _Hack_VFS = nullptr;
    return nullptr;
  }

  auto PCHContainerOps = std::make_shared<PCHContainerOperations>();

  /* Hacked function call, see ASTUnitHack.cpp.  */
  auto AU = ASTUnit::create(ClangCompat_ASTUP(CInvok,
                                              diagopts,
                                              Diags,
                                              CaptureDiagsKind::None,
                                              false));
  std::unique_ptr<ASTUnit> *ErrAST = nullptr;

  ASTUnit::LoadFromCompilerInvocationAction(ClangCompat_ASTULFCIAP(
                                            CInvok, PCHContainerOps,
                                            diagopts,
                                            Diags,
                                            nullptr, AU.get(),
                                            /*Persistent=*/true,
                                            /*ResourceFilesPath=*/StringRef(),
                                            /*OnlyLocalDecls=*/false,
                                            /*CaptureDiagnostics=*/CaptureDiagsKind::None,
                                            /*PrecompilePreambleAfterNParses=*/0,
                                            /*CacheCodeCompletionResults=*/false,
                                            /*UserFilesAreVolatile=*/false,
                                            ErrAST));

  _Hack_VFS = nullptr;

  if (AU == nullptr) {
    if (ignore_clang_errors && ErrAST) {
      return std::unique_ptr<ASTUnitWrapper>(new ASTUnitWrapper(std::move(*ErrAST)));
    }

    throw std::runtime_error("Unable to create ASTUnit object.");
  }

  return std::unique_ptr<ASTUnitWrapper>(new ASTUnitWrapper(std::move(AU)));
}

bool ASTUnitWrapper::Reparse(IntrusiveRefCntPtr<llvm::vfs::FileSystem> fs)
{
  /* ASTUnit::Reparse returns true on error.  */
  return !AST->Reparse(std::make_shared<PCHContainerOperations>(), {}, fs);
}

/* ---- Hacky code so we can create an ASTUnit with a custom filesystem. ---- */

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
