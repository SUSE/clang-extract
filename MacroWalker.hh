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


  /** Check if IdentifierInfo `tok` is actually an argument of the macro info.  */
  static bool Is_Identifier_Macro_Argument(MacroInfo *info, const IdentifierInfo *tok);

  private:
  Preprocessor &PProcessor;
};
