/** Header providing compatibility with older versions of clang.
 *
 * Clang API often have minor changes across versions. For this tool to be
 * compatible with multiple versions, we have to provide a minimal interface
 * between them.
 */

#pragma once

#include <clang/Tooling/Tooling.h>
#include <clang/Basic/Version.h>
#include "clang/Frontend/CompilerInstance.h"

namespace ClangCompat
{
#if CLANG_VERSION_MAJOR >= 16
# define ClangCompat_None std::nullopt
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
    clang::CreateInvocationOptions CIOpts;
    CIOpts.Diags = Diags;
    return clang::createInvocation(Args, std::move(CIOpts));
#else
    return clang::createInvocationFromCommandLine(Args, Diags);
#endif
  }

  static inline const Type *getTypePtr(const QualType &qtype)
  {
#if CLANG_VERSION_MAJOR >= 17
    /* Starting from clang-17 it crashes if qtype isNull is true.  */
    return qtype.isNull() ? nullptr : qtype.getTypePtr();
#else
    return qtype.getTypePtr();
#endif
  }
}
