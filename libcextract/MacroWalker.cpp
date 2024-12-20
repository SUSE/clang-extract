//===- MacroWalker.cpp - Get macro structures according to location *- C++ -*-===//
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

#include "MacroWalker.hh"
#include "PrettyPrint.hh"

MacroInfo *MacroWalker::Get_Macro_Info(MacroDefinitionRecord *record)
{
  MacroDirective *directive = Get_Macro_Directive(record);
  return directive ? directive->getMacroInfo() : nullptr;
}

MacroInfo *MacroWalker::Get_Macro_Info(MacroExpansion *macroexp)
{
  SourceLocation loc = macroexp->getSourceRange().getBegin();
  const IdentifierInfo *id = macroexp->getName();
  return Get_Macro_Info(id, loc);
}

MacroInfo *MacroWalker::Get_Macro_Info(const IdentifierInfo *id, const SourceLocation &loc)
{
  MacroDirective *directive = PProcessor.getLocalMacroDirectiveHistory(id);
  while (directive) {
    MacroInfo *macroinfo = directive->getMacroInfo();

    if (!macroinfo->getDefinitionLoc().isValid()) {
      /* Skip if macro expansion seems to not have a valid location.  */
      directive = directive->getPrevious();
      continue;
    }

    /* If this MacroInfo object location is defined AFTER the given loc, then
       it means we are looking for a macro that was later redefined.  So we
       look into the previous definitions to find the last one that is defined
       before loc.  */

    if (PrettyPrint::Is_Before(macroinfo->getDefinitionLoc(), loc))
      return macroinfo;

    directive = directive->getPrevious();
  }

  /* MacroInfo object not found.  */
  return nullptr;
}

MacroDirective *MacroWalker::Get_Macro_Directive(MacroDefinitionRecord *record)
{
  const IdentifierInfo *id = record->getName();
  MacroDirective *directive = PProcessor.getLocalMacroDirectiveHistory(id);
  while (directive) {
    MacroInfo *macroinfo = directive->getMacroInfo();

    /* If this MacroInfo object location is defined OUTSIDE the macro defintion,
       then it means we are looking for a macro that was later redefined.  */
    SourceRange range1(macroinfo->getDefinitionLoc(), macroinfo->getDefinitionEndLoc());
    SourceRange range2(record->getLocation());
    if (range1.fullyContains(range2)) {
      return directive;
    }
    directive = directive->getPrevious();
  }

  return nullptr;
}

bool MacroWalker::Is_Builtin_Macro(MacroInfo *info)
{
  if (info->isBuiltinMacro())
    return true;

  /* Some cases the isBuiltinMacro method fails on builtin macros.  Try to
     decide it using the FileInfo.  */
  OptionalFileEntryRef ref = PrettyPrint::Get_FileEntry(info->getDefinitionLoc());
  if (!ref.has_value()) {
    /* This doesn't come from any file, hence it was introduced by the compiler,
       therefore is a builtin macro.  */
    return true;
  }

  /* This comes from a file and therefore is not a builtin macro.  */
  return false;
}

bool MacroWalker::Is_Identifier_Macro_Argument(MacroInfo *info, StringRef tok_str)
{
  for (const IdentifierInfo *arg : info->params()) {
    StringRef arg_str = arg->getName();
    if (tok_str == arg_str) {
      return true;
    }
  }

  return false;
}

bool MacroWalker::Is_Identifier_Macro_Argument(MacroInfo *info, const IdentifierInfo *tok)
{
  if (tok == nullptr)
    return false;

  return Is_Identifier_Macro_Argument(info, tok->getName());
}
