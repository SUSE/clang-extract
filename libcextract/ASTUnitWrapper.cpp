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
static IntrusiveRefCntPtr<llvm::vfs::FileSystem> _Hack_VFS;

ASTUnitWrapper *ASTUnitWrapper::Last_Created = nullptr;

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

StringRef ASTUnitWrapper::Get_Source_Text(const SourceRange &range)
{
    // NOTE: sm.getSpellingLoc() used in case the range corresponds to a macro/preprocessed source.
    // NOTE2: getSpellingLoc() breaks in the case where a macro was asigned to be expanded to typedef.
    SourceManager &SM = AST->getSourceManager();
    const LangOptions &langopts = AST->getLangOpts();
    auto start_loc = range.getBegin();//SM->getSpellingLoc(range.getBegin());
    auto last_token_loc = range.getEnd();//SM->getSpellingLoc(range.getEnd());
    auto end_loc = clang::Lexer::getLocForEndOfToken(last_token_loc, 0, SM, langopts);
    auto printable_range = clang::SourceRange{start_loc, end_loc};
    return Get_Source_Text_Raw(printable_range);
}

StringRef ASTUnitWrapper::Get_Source_Text_Raw(const SourceRange &range)
{
    SourceManager &SM = AST->getSourceManager();
    const LangOptions &langopts = AST->getLangOpts();
    return clang::Lexer::getSourceText(CharSourceRange::getCharRange(range), SM, langopts);
}

/* Get the TopLevel Decl that contains the location loc.  */
Decl *ASTUnitWrapper::Get_Toplevel_Decl_At_Location(const SourceLocation &loc)
{
  SourceManager &SM = AST->getSourceManager();
  /* We don't have a way of accessing the TopLevel vector directly, hence we
     do this.  */
  char *p = (char *) &(*AST->top_level_begin());
  char *q = (char *) &(*AST->top_level_end());

  int n = (((ptrdiff_t)(q - p))/sizeof(Decl *));

  Decl **array = (Decl **)p;

  /* Do binary search.  */
  int low = 0;
  int high = n-1;
  while (low <= high) {
    int mid = low + (high - low)/2;

    Decl *decl = array[mid];
    /* Get rid of some weird macro locations.  We want the location where
       it was expanded.  */
    SourceRange decl_range(SM.getExpansionLoc(decl->getBeginLoc()),
                           SM.getExpansionLoc(decl->getEndLoc()));

    if (decl_range.fullyContains(loc)) {
      return decl;
    }

    if (SM.isBeforeInTranslationUnit(decl_range.getBegin(), loc)) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }

  return nullptr;
}

ArrayRef<Decl *> ASTUnitWrapper::Get_Toplev_Decls_With_Same_Beginloc(const SourceLocation &loc)
{
  SourceManager &SM = AST->getSourceManager();
  /* We don't have a way of accessing the TopLevel vector directly, hence we
     do this.  */
  char *p = (char *) &(*AST->top_level_begin());
  char *q = (char *) &(*AST->top_level_end());

  int n = (((ptrdiff_t)(q - p))/sizeof(Decl *));

  Decl **array = (Decl **)p;

  /* Do binary search.  */
  int low = 0;
  int high = n-1;
  while (low <= high) {
    int mid = low + (high - low)/2;

    Decl *decl = array[mid];
    /* Get rid of some weird macro locations.  We want the location where
       it was expanded.  */
    SourceLocation begin = SM.getExpansionLoc(decl->getBeginLoc());

    if (begin == loc) {
      /* Go back to the first declaration.  */
      int last_l = mid;
      for (int l = mid; l >= 0; --l) {
        if (SM.getExpansionLoc(array[l]->getBeginLoc()) == loc) {
          last_l = l;
        } else {
          break;
        }
      }

      /* Go forward to the last declaration.  */
      int last_h = mid;
      for (int h = mid; h < n; ++h) {
        if (SM.getExpansionLoc(array[h]->getBeginLoc()) == loc) {
          last_h = h;
        } else {
          break;
        }
      }

      return ArrayRef(&array[last_l], &array[last_h+1]);
    }

    if (SM.isBeforeInTranslationUnit(begin, loc)) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }

  return ArrayRef<Decl *>(nullptr, size_t(0));
}

/** Lookup in the symbol table for a declaration with given name passed by info.  */
DeclContextLookupResult ASTUnitWrapper::Get_Decl_From_Symtab(const IdentifierInfo *info)
{
  TranslationUnitDecl *tu = AST->getASTContext().getTranslationUnitDecl();
  return tu->lookup(DeclarationName(info));
}

/** Lookup in the symbol table for a declaration with given name passed by name.  */
DeclContextLookupResult ASTUnitWrapper::Get_Decl_From_Symtab(const StringRef &name)
{
  IdentifierTable &symtab = AST->getPreprocessor().getIdentifierTable();
  auto info = symtab.find(name);
  /* Return an empty DeclContextLookupResult if the identifier is not found */
  if (info == symtab.end())
    return DeclContextLookupResult();

  return Get_Decl_From_Symtab(info->getValue());
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
