#pragma once

#include <clang/Tooling/Tooling.h>
#include <unordered_set>

using namespace clang;

class MacroDependencyFinder
{
  public:
  MacroDependencyFinder(Preprocessor &p);

  void Print_Macros_Before(const SourceLocation &loc);

  void Print_Macros_Between(const SourceLocation &begin, const SourceLocation &end);
  void Print_Macros_Between(const SourceRange &range);

  void Print_Macros_After(const SourceLocation &loc);

  void Find_Macros_Required(ASTUnit *ast);

  private:

  Preprocessor &PProcessor;

  std::unordered_set<MacroDefinitionRecord*> Dependencies;

  inline bool Is_Macro_Marked(MacroDefinitionRecord *prerec)
  { return Dependencies.find(prerec) != Dependencies.end(); }
};
