#include "MacroDepsFinder.hh"
#include "FunctionDepsFinder.hh"
#include "PrettyPrint.hh"
#include <iostream>


MacroDependencyFinder::MacroDependencyFinder(Preprocessor &p)
  : PProcessor(p)
{
}

MacroIterator MacroDependencyFinder::Get_Macro_Iterator(void)
{
  MacroIterator it;
  it.macro_it = PProcessor.getPreprocessingRecord()->begin();
  it.undef_it = 0;
  return it;
}

void MacroDependencyFinder::Skip_Macros_Until(MacroIterator &it, const SourceLocation &loc)
{
  Print_Macros_Until(it, loc, /*print = */ false);
}

void MacroDependencyFinder::Print_Macros_Until(MacroIterator &it, const SourceLocation &loc, bool print)
{
  PreprocessingRecord *rec = PProcessor.getPreprocessingRecord();

  /* Ensure that PreprocessingRecord is avaialable.  */
  assert(rec && "PreprocessingRecord wasn't generated");

  SourceLocation curr_loc;

  /* We have to iterate in both the preprocessor record and the marked undef
     vector to find which one we should print based on the location.  This is
     somewhat similar to how mergesort merge two sorted vectors, if this
     somehow helps to understand this code.  */
  while (true) {
    PreprocessedEntity *e = (it.macro_it == rec->end())? nullptr: *it.macro_it;
    MacroDirective *undef = nullptr;
    bool should_print_undef = false;

    /* If we still have items to iterate on the preprocessing record.  */
    if (e) {
      curr_loc = e->getSourceRange().getBegin();
    }

    /* If we still have itens to iterate on the Undef vector.  */
    if (it.undef_it < NeedsUndef.size()) {
      undef = NeedsUndef[it.undef_it];
      SourceLocation undef_loc = undef->getDefinition().getUndefLocation();

      /* Now find out which one comes first.  */
      if (!e || PrettyPrint::Is_Before(undef_loc, curr_loc)) {
        should_print_undef = true;
        curr_loc = undef_loc;
      }
    }

    /* Check if we did not past the loc marker where we should stop printing.  */
    if (PrettyPrint::Is_Before(curr_loc, loc)) {
      if (should_print_undef) {
        if (print)
          PrettyPrint::Print_Macro_Undef(undef);
        it.undef_it++;
      } else if (e) {
        MacroDefinitionRecord *entity = dyn_cast<MacroDefinitionRecord>(e);
        if (print && entity && Is_Macro_Marked(Get_Macro_Info(entity))) {
          PrettyPrint::Print_Macro_Def(entity);
        }
        it.macro_it++;
      } else {
        break;
      }
    } else {
      break;
    }

  }
}

void MacroDependencyFinder::Print_Remaining_Macros(MacroIterator &it)
{
  SourceLocation loc = SourceLocation::getFromRawEncoding(UINT_MAX);
  Print_Macros_Until(it, loc, true);
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
  MacroDirective *directive = Get_Macro_Directive(record);
  return directive ? directive->getMacroInfo() : nullptr;
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

MacroDirective *MacroDependencyFinder::Get_Macro_Directive(MacroDefinitionRecord *record) {
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

int MacroDependencyFinder::Populate_Need_Undef(void)
{
  int count = 0;
  PreprocessingRecord *rec = PProcessor.getPreprocessingRecord();

  for (PreprocessedEntity *entity : *rec) {
    if (MacroDefinitionRecord *def = dyn_cast<MacroDefinitionRecord>(entity)) {

      MacroDirective *directive = Get_Macro_Directive(def);
      assert(directive);

      /* There is no point in analyzing a macro that wasn't added for output.  */
      if (Is_Macro_Marked(directive->getMacroInfo())) {
        SourceLocation undef_loc = directive->getDefinition().getUndefLocation();

        /* In case the macro isn't undefined then its location is invalid.  */
        if (undef_loc.isValid()) {
          NeedsUndef.push_back(directive);
          count++;
        }
      }
    }
  }

  return count;
}

void MacroDependencyFinder::Find_Macros_Required(FunctionDependencyFinder *fdf, ASTUnit *ast)
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

  clang::ASTUnit::top_level_iterator it = ast->top_level_begin();
  clang::PreprocessingRecord::iterator macro_it = rec->begin();

  while (it != ast->top_level_end()) {
    Decl *decl = *it;

    if (decl && fdf->Is_Decl_Marked(decl)) {
      PreprocessedEntity *entity = *macro_it;

      /* While the macro is before the end limit of the decl range, then:  */
      while (macro_it != rec->end() && PrettyPrint::Is_Before(entity->getSourceRange().getBegin(), decl->getEndLoc())) {

        /* If the macro location is in the Decl (function, variable, ...) range,
           then analyze this macro and its dependencies.  */
        if (decl->getSourceRange().fullyContains(entity->getSourceRange())) {
          if (MacroExpansion *macroexp = dyn_cast<MacroExpansion>(entity)) {
            Backtrack_Macro_Expansion(macroexp);
          }
        }

        entity = *(++macro_it);
      }
    }
    it++;
  }

  Populate_Need_Undef();
}

void MacroDependencyFinder::Dump_Dependencies(void)
{
  std::unordered_set<MacroInfo*>::iterator itr;
  for (itr = Dependencies.begin(); itr != Dependencies.end(); itr++) {
    PrettyPrint::Print_MacroInfo(*itr);
    std::cout << '\n';
  }
}
