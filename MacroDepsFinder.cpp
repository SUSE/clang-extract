#include "MacroDepsFinder.hh"
#include "PrettyPrint.hh"
#include <iostream>

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
      MacroInfo *info = Get_Macro_Info(entity);
      if (Is_Macro_Marked(info)) {
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
      MacroInfo *info = Get_Macro_Info(macro);
      if (info && Is_Macro_Marked(info)) {
        PrettyPrint::Print_Macro_Def(macro);
      }
    }
  }
}

bool MacroDependencyFinder::Backtrack_Macro_Expansion(MacroExpansion *macroexp)
{
  MacroInfo *info = Get_Macro_Info(macroexp);
  return Backtrack_Macro_Expansion(info, macroexp->getSourceRange().getBegin());
}

/** Check if IdentifierInfo `tok` is actually an argument of the macro info.  */
static bool Is_Identifier_Macro_Argument(MacroInfo *info, const IdentifierInfo *tok)
{

  StringRef tok_str = tok->getName();
  for (const IdentifierInfo *arg : info->params()) {
    StringRef arg_str = arg->getName();
    if (tok_str.equals(arg_str)) {
      return true;
    }
  }

  return false;
}

bool MacroDependencyFinder::Backtrack_Macro_Expansion(MacroInfo *info, const SourceLocation &loc)
{
  bool inserted = false;

  if (info == nullptr)
    return false;

  /* If this MacroInfo object with the macro contents wasn't marked for output
     then mark it now.  */
  if (!Is_Macro_Marked(info)) {
    Dependencies.insert(info);
    inserted = true;
  }

  /* We can not quickly return if the macro is already marked.  Other macros
     which this macro depends on may have been redefined in meanwhile, and
     we don't implement some dependency tracking so far, so redo the analysis.  */

  clang::MacroInfo::tokens_iterator it;
  /* Iterate on the expansion tokens of this macro to find if it references other
     macros.  */
  for (it = info->tokens_begin(); it != info->tokens_end(); ++it) {
    const Token *tok = it;
    const IdentifierInfo *tok_id = tok->getIdentifierInfo();

    if (tok_id != nullptr) {
      MacroInfo *maybe_macro = Get_Macro_Info(tok->getIdentifierInfo(), loc);

      /* We must be careful to not confuse tokens which are function-like macro
         arguments with other macors.  Example:

         #define MAX(a, b) ((a) > (b) ? (a) : (b))
         #define a 9

         We should not add `a` as macro when analyzing MAX once it is clearly an
         argument of the macro, not a reference to other symbol.  */
      if (maybe_macro && !Is_Identifier_Macro_Argument(info, tok_id)) {
        inserted |= Backtrack_Macro_Expansion(maybe_macro, loc);
      }
    }
  }

  return inserted;
}

MacroInfo *MacroDependencyFinder::Get_Macro_Info(MacroDefinitionRecord *record)
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
      return macroinfo;
    }
    directive = directive->getPrevious();
  }

  return nullptr;
}

MacroInfo *MacroDependencyFinder::Get_Macro_Info(MacroExpansion *macroexp)
{
  SourceLocation loc = macroexp->getSourceRange().getBegin();
  const IdentifierInfo *id = macroexp->getName();
  return Get_Macro_Info(id, loc);
}

MacroInfo *MacroDependencyFinder::Get_Macro_Info(const IdentifierInfo *id, const SourceLocation &loc)
{
  MacroDirective *directive = PProcessor.getLocalMacroDirectiveHistory(id);
  while (directive) {
    MacroInfo *macroinfo = directive->getMacroInfo();

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

  /* For now add all macros that are expanded.  We need a proper analysis about
     which macros are necessary.  */
  for (PreprocessedEntity *entity : *rec) {
    if (MacroExpansion *macroexp = dyn_cast<MacroExpansion>(entity)) {
      Backtrack_Macro_Expansion(macroexp);
    }
  }
}

void MacroDependencyFinder::Dump_Dependencies(void)
{
  std::unordered_set<MacroInfo*>::iterator itr;
  for (itr = Dependencies.begin(); itr != Dependencies.end(); itr++) {
    PrettyPrint::Print_MacroInfo(*itr);
    std::cout << '\n';
  }
}
