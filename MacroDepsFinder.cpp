#include "MacroDepsFinder.hh"
#include "PrettyPrint.hh"

MacroDependencyFinder::MacroDependencyFinder(Preprocessor &p)
  : PProcessor(p)
{
}

void MacroDependencyFinder::Skip_Macros_Until(MacroIterator &it, const SourceLocation &loc)
{
  PreprocessingRecord *rec = PProcessor.getPreprocessingRecord();

  /* Ensure that PreprocessingRecord is avaialable.  */
  assert(rec && "PreprocessingRecord wasn't generated");

  for (; it != rec->end(); ++it) {
    MacroDefinitionRecord *entity = dyn_cast<MacroDefinitionRecord>(*it);

    if (!entity || PrettyPrint::Is_Before(entity->getLocation(), loc)) {
      continue;
    } else {
      /* We passed the location marker loc.  */
      break;
    }
  }
}

void MacroDependencyFinder::Print_Macros_Until(MacroIterator &it, const SourceLocation &loc)
{
  PreprocessingRecord *rec = PProcessor.getPreprocessingRecord();

  /* Ensure that PreprocessingRecord is avaialable.  */
  assert(rec && "PreprocessingRecord wasn't generated");

  for (; it != rec->end(); ++it) {
    MacroDefinitionRecord *entity = dyn_cast<MacroDefinitionRecord>(*it);

    /* If entity is not a vailid MacroDefinitionRecord then skip to next.  */
    if (!entity) {
      continue;
    }

    if (PrettyPrint::Is_Before(entity->getLocation(), loc)) {
      if (Is_Macro_Marked(entity)) {
        PrettyPrint::Print_Macro_Def(entity);
      }
    } else {
      /* We passed the location marker loc.  */
      break;
    }
  }
}

void MacroDependencyFinder::Print_Remaining_Macros(MacroIterator &it)
{
  PreprocessingRecord *rec = PProcessor.getPreprocessingRecord();

  /* Ensure that PreprocessingRecord is avaialable.  */
  assert(rec && "PreprocessingRecord wasn't generated");

  for (; it != rec->end(); ++it) {
    PreprocessedEntity *entity = *it;

    if (MacroDefinitionRecord *macro = dyn_cast<MacroDefinitionRecord>(entity)) {
      if (Is_Macro_Marked(macro)) {
        PrettyPrint::Print_Macro_Def(macro);
      }
    }
  }
}

void MacroDependencyFinder::Find_Macros_Required(ASTUnit *ast)
{
  /* If a SourceManager wasn't passed to the PrettyPrint class we cannot
     continue.  */
  if (PrettyPrint::Get_Source_Manager() == nullptr) {
    return;
  }

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
