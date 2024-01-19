//===- MacroWalker.hh - Get macro structures according to location *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Helper functions and methods to handle macros.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#pragma once

#include <clang/Tooling/Tooling.h>

using namespace clang;

class MacroWalker
{
  public:
  MacroWalker(Preprocessor &p)
    : PProcessor(p)
  {
  }

  /** The object which holds the macro arguments is the MacroInfo.  Get the
      MacroInfo being extra careful to not pick the last definition, since
      macros can be redefined.  */
  MacroInfo *Get_Macro_Info(MacroDefinitionRecord *record);
  MacroInfo *Get_Macro_Info(MacroExpansion *macroexp);
  MacroInfo *Get_Macro_Info(const IdentifierInfo *id, const SourceLocation &loc);

  MacroDirective* Get_Macro_Directive(MacroDefinitionRecord *record);

  /** Check if macro is a builtin-macro.  */
  bool Is_Builtin_Macro(MacroInfo *info);

  /** Check if IdentifierInfo `tok` is actually an argument of the macro info.  */
  static bool Is_Identifier_Macro_Argument(MacroInfo *info, const IdentifierInfo *tok);
  static bool Is_Identifier_Macro_Argument(MacroInfo *info, StringRef tok_str);

  private:
  Preprocessor &PProcessor;
};
