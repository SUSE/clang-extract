#include "MacroDepsFinder.hh"
#include "PrettyPrint.hh"

MacroDependencyFinder::MacroDependencyFinder(Preprocessor &p)
  : PProcessor(p)
{
}

void MacroDependencyFinder::Print_Macros_Before(const SourceLocation &loc)
{
  return;
  PreprocessingRecord *rec = PProcessor.getPreprocessingRecord();

  /* Ensure that PreprocessingRecord is avaialable.  */
  assert(rec && "PreprocessingRecord wasn't generated");

  for (PreprocessedEntity *entity : *rec) {
    if (MacroDefinitionRecord *macro = dyn_cast<MacroDefinitionRecord>(entity)) {

      /* If we already passed the loc then quickly return.  */
      if (PrettyPrint::Is_Before(loc, macro->getLocation())) {
        return;
      }

      if (PrettyPrint::Is_Before(macro->getLocation(), loc)) {
        if (Is_Macro_Marked(macro)) {
          PrettyPrint::Print_Macro_Def(macro);
        }
      }
    }
  }
}

void MacroDependencyFinder::Print_Macros_Between(const SourceLocation &begin, const SourceLocation &end)
{
  SourceRange range(begin, end);
  Print_Macros_Between(range);
}

void MacroDependencyFinder::Print_Macros_Between(const SourceRange &range)
{
  return;
  PreprocessingRecord *rec = PProcessor.getPreprocessingRecord();

  /* Ensure that PreprocessingRecord is avaialable.  */
  assert(rec && "PreprocessingRecord wasn't generated");

  for (PreprocessedEntity *entity : rec->getPreprocessedEntitiesInRange(range)) {
    MacroDefinitionRecord *macro;
    if (entity && (macro = dyn_cast<MacroDefinitionRecord>(entity))) {
      if (Is_Macro_Marked(macro)) {
        PrettyPrint::Print_Macro_Def(macro);
      }
    }
  }
}

void MacroDependencyFinder::Print_Macros_After(const SourceLocation &loc)
{
  return;
  PreprocessingRecord *rec = PProcessor.getPreprocessingRecord();

  /* Ensure that PreprocessingRecord is avaialable.  */
  assert(rec && "PreprocessingRecord wasn't generated");

  for (PreprocessedEntity *entity : *rec) {
    if (MacroDefinitionRecord *macro = dyn_cast<MacroDefinitionRecord>(entity)) {
      if (PrettyPrint::Is_Before(loc, macro->getLocation())) {
        if (Is_Macro_Marked(macro)) {
          PrettyPrint::Print_Macro_Def(macro);
        }
      }
    }
  }
}

void MacroDependencyFinder::Find_Macros_Required(ASTUnit *ast)
{
  PreprocessingRecord *rec = PProcessor.getPreprocessingRecord();

  /* Ensure that PreprocessingRecord is avaialable.  */
  assert(rec && "PreprocessingRecord wasn't generated");

  /* Ensure that the Preprocessor object in this class is the same as the one
     in the given ast.  */
  assert(&PProcessor == &ast->getPreprocessor());

  /* For now add all macros as required.  We need a proper analysis about
     which macros are necessary.  */
  for (PreprocessedEntity *entity : *rec) {
    if (MacroDefinitionRecord *macro = dyn_cast<MacroDefinitionRecord>(entity)) {
      PrettyPrint::Print_Macro_Def(macro);
      if (!Is_Macro_Marked(macro)) {
        Dependencies.insert(macro);
      }
    }
  }
}
