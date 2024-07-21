/*
 *  clang-extract - Extract functions from projects and its dependencies using
 *                  libclang and LLVM infrastructure.
 *
 *
 *  This file is part of clang-extract.
 *
 *  clang-extract is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  clang-extract is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with clang-extract.  If not, see <http://www.gnu.org/licenses/>.
 */


/** Header providing compatibility with older versions of clang.
 *
 * Clang API often have minor changes across versions. For this tool to be
 * compatible with multiple versions, we have to provide a minimal interface
 * between them.
 *
 * Author: Giuliano Belinassi
 */

#pragma once

#include <clang/Tooling/Tooling.h>
#include <clang/Basic/Version.h>
#include "clang/Frontend/CompilerInstance.h"

/* Starting from LLVM-18, the method FileEntry::getName() got deprecated.
 * This makes clang warns about the function being deprecated while it is
 * completely useful as a way to keep compatibility with older versions of
 * LLVM, hence we acknowledge the warning here and disable it for now.
 */

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

namespace ClangCompat
{
#if CLANG_VERSION_MAJOR >= 16
/** Some functions changed its signature to expect std::nullopt when no option is given.*/
# define ClangCompat_None std::nullopt
/** Clang do not provide getPointer() for tokens anymore.  */
# define ClangCompat_GetTokenPtr(token) (token).has_value() ? &(token).value() : nullptr
#else
# define ClangCompat_None clang::None
# define ClangCompat_GetTokenPtr(token) (token).getPointer()
#endif

  static inline const clang::TypedefType *Get_Type_As_TypedefType(const clang::Type *type)
  {
#if CLANG_VERSION_MAJOR >= 16
      /* Clang 16 upwards do not provide getAs<const TypedefType>.  */
    return type->getAs<clang::TypedefType>();
#else
    return type->getAs<const clang::TypedefType>();
#endif
  }

  static inline auto
  createInvocationFromCommandLine(ArrayRef<const char *> Args, IntrusiveRefCntPtr<DiagnosticsEngine> Diags=IntrusiveRefCntPtr< DiagnosticsEngine >())
  {
#if CLANG_VERSION_MAJOR >= 15
    /* Provide an implementation of createInvocationFromCommandLine for newer versions of clang.  */
    clang::CreateInvocationOptions CIOpts;
    CIOpts.Diags = Diags;
    return clang::createInvocation(Args, std::move(CIOpts));
#else
    return clang::createInvocationFromCommandLine(Args, Diags);
#endif
  }

  static inline const clang::Type *getTypePtr(const QualType &qtype)
  {
#if CLANG_VERSION_MAJOR >= 17
    /* Starting from clang-17 it crashes if qtype isNull is true.  */
    return qtype.isNull() ? nullptr : qtype.getTypePtr();
#else
    return qtype.getTypePtr();
#endif
  }

  static inline auto Get_Main_Directory_Arr(const SourceManager &sm)
  {
    /* This function is used to provide a valid object for the function
       `LookupFile`, which do not seems to provide a stable interface between
       versions.  */
    FileID main_file = sm.getMainFileID();
    OptionalFileEntryRef main_fentry = sm.getFileEntryRefForID(main_file);
    DirectoryEntryRef dir_ref = (*main_fentry).getDir();

#if CLANG_VERSION_MAJOR >= 18
    std::array<std::pair<OptionalFileEntryRef, DirectoryEntryRef>, 1> A {{
      { main_fentry, dir_ref },
    }};
    return A;
#elif CLANG_VERSION_MAJOR == 17
    const FileEntry *fentry      = &(*main_fentry).getFileEntry();

    std::array<std::pair<const FileEntry *, DirectoryEntryRef>, 1> A {{
      { fentry, dir_ref },
    }};
    return A;
#else
    const FileEntry *fentry      = &(*main_fentry).getFileEntry();
    const DirectoryEntry *dentry = &dir_ref.getDirEntry();

    std::array<std::pair<const FileEntry *, const DirectoryEntry *>, 1> A {{
      { fentry, dentry },
    }};
    return A;
#endif
  }
}
