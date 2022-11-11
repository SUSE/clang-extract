#include "MacroDepsFinder.hh"
#include "FunctionDepsFinder.hh"
#include "PrettyPrint.hh"
#include <iostream>

#define PProcessor (AST->getPreprocessor())

MacroDependencyFinder::MacroDependencyFinder(ASTUnit *ast, std::string const &function)
  : FunctionDependencyFinder(ast, function),
    MW(ast->getPreprocessor())
{
  /* The call to the parent constructor builds the dependency set of functions
     and types.  Now build the macro dependency list.  */
  Find_Macros_Required();
}

MacroDependencyFinder::MacroDependencyFinder(ASTUnit *ast, std::vector<std::string> const &functions)
  : FunctionDependencyFinder(ast, functions),
    MW(ast->getPreprocessor())
{
  Find_Macros_Required();
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
    if (curr_loc.isValid() && PrettyPrint::Is_Before(curr_loc, loc)) {
      if (should_print_undef) {
        if (print)
          PrettyPrint::Print_Macro_Undef(undef);
        it.undef_it++;
      } else if (e) {
        MacroDefinitionRecord *entity = dyn_cast<MacroDefinitionRecord>(e);
        if (print && entity && Is_Macro_Marked(MW.Get_Macro_Info(entity))) {
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
    if (should_print_undef) {
      PrettyPrint::Print_Macro_Undef(undef);
      it.undef_it++;
    } else if (e) {
      MacroDefinitionRecord *entity = dyn_cast<MacroDefinitionRecord>(e);
      if (entity && Is_Macro_Marked(MW.Get_Macro_Info(entity))) {
        PrettyPrint::Print_Macro_Def(entity);
      }
      it.macro_it++;
    } else {
      break;
    }
  }
}

void MacroDependencyFinder::Print(void)
{
  SourceLocation last_decl_loc = SourceLocation::getFromRawEncoding(0U);
  bool first = true;

  /* We can only print macros if we have a SourceManager.  */
  bool can_print_macros = (PrettyPrint::Get_Source_Manager() != nullptr);

  MacroIterator macro_it = Get_Macro_Iterator();
  clang::ASTUnit::top_level_iterator it = AST->top_level_begin();
  for (it = AST->top_level_begin(); it != AST->top_level_end(); ++it) {
    Decl *decl = *it;

    if (Is_Decl_Marked(decl)) {
      if (can_print_macros) {
        /* In the first decl we don't have a last source location, hence we have
           to handle this special case.  */
        if (first) {
          Print_Macros_Until(macro_it, decl->getBeginLoc());
          first = false;
        } else {
          /* Macros defined inside a function is printed together with the function,
             so we must skip them.  */
          Skip_Macros_Until(macro_it, last_decl_loc);

          Print_Macros_Until(macro_it, decl->getBeginLoc());
        }
      }
      last_decl_loc = decl->getEndLoc();
      PrettyPrint::Print_Decl(decl);
    }
  }

  if (can_print_macros) {
    Skip_Macros_Until(macro_it, last_decl_loc);
    /* Print remaining macros.  */
    Print_Remaining_Macros(macro_it);
  }
}

/** Set containing the macros which were already analyzed.  */
static std::unordered_set<MacroInfo *> AnalyzedMacros;
static bool Already_Analyzed(MacroInfo *info)
{
  return AnalyzedMacros.find(info) != AnalyzedMacros.end();
}

/** Get next token in the token list.  */
static StringRef Consume_Token(StringRef &token_list)
{
  static const char *empty = "";
  int leading_spaces = 0;

  if (token_list.size() == 0) {
    return StringRef(empty);
  }

  /* Consume the spaces, if any.  */
  while (token_list[leading_spaces] == ' ') {
    leading_spaces++;
  }

  token_list = token_list.drop_front(leading_spaces);

  /* Find , token argument separator.  */
  size_t comma = token_list.find_first_of(',');

  StringRef token = token_list.substr(0, comma);
  if (comma < token_list.npos) {
    token_list = token_list.drop_front(comma+1);
  } else {
    token_list = StringRef(empty);
  }

  return token;
}

bool MacroDependencyFinder::Backtrack_Macro_Expansion(MacroExpansion *macroexp)
{
  MacroInfo *info = MW.Get_Macro_Info(macroexp);
  std::map<std::string, std::string> symtab;

  /* In the case the macro is function-like, we must find the tokens which are
     they parameters.  */
  if (info && info->isFunctionLike()) {
    StringRef expansion_str = PrettyPrint::Get_Source_Text(macroexp->getSourceRange());

    /* Find parameters.  */
    size_t lparen = expansion_str.find_first_of('(');
    size_t rparen = expansion_str.find_last_of(')');
    size_t len = rparen - lparen - 1;

    assert(len >= 0);

    StringRef token_list = expansion_str.substr(lparen+1, len);

    auto it = info->param_begin();
    StringRef token = Consume_Token(token_list);
    while (token != "" && it != info->param_end()) {

      std::string var = (*it)->getName().str();
      symtab[var] = token.str();

      token = Consume_Token(token_list);
      it++;
    }
  }

  /* Reset the analyzed set.  */
  AnalyzedMacros = std::unordered_set<MacroInfo *>();
  return Backtrack_Macro_Expansion(info, macroexp->getSourceRange().getBegin(), symtab);
}

static StringRef Get_Token_String(const Token *tok)
{
  tok::TokenKind tk = tok->getKind();

  if (tok::isAnyIdentifier(tk)) {
    return tok->getIdentifierInfo()->getName();
  }

  const char *punct = tok::getPunctuatorSpelling(tk);
  if (punct) {
    return StringRef(punct);
  }

  const char *spell = tok::getKeywordSpelling(tk);
  if (punct) {
    return StringRef(spell);
  }

  return StringRef();
}


void Dump_Map(std::map<std::string, std::string> &map)
{
  for (const auto& elem : map)
  {
    llvm::outs() << elem.first << " : " << elem.second << "\n";
  }
}

static std::map<std::string, std::string> Get_Macro_Args_Replacement(MacroInfo *info, MacroInfo::const_tokens_iterator curr_it)
{
  std::map<std::string, std::string> symtab;

  if (info->isFunctionLike()) {
    int num_paren = 0;
    const Token *tok;
    MacroInfo::param_iterator param = info->param_begin();

    do {
      tok = ++curr_it;
      tok::TokenKind tok_kind = tok->getKind();

      if (tok_kind == tok::l_paren) {
        num_paren++;
      } else if (tok_kind == tok::r_paren) {
        num_paren--;
      }

      IdentifierInfo *arg_info = tok->getIdentifierInfo();
      if (num_paren == 1 && tok_kind != tok::comma && arg_info) {
        StringRef arg_str = arg_info->getName();
        const StringRef param_str = (*param)->getName();

        symtab[param_str.str()] = arg_str.str();

        llvm::outs() << param_str << " : " << arg_str << '\n';
        param++;
      }
    } while (num_paren > 0);
  }

  return symtab;
}

bool MacroDependencyFinder::Backtrack_Macro_Expansion(MacroInfo *info, const SourceLocation &loc, std::map<std::string, std::string> symtab)
{
  bool inserted = false;

  if (info == nullptr)
    return false;

  /* If this macro were already been analyzed, then quickly return.

     We can not use the MacroDependency set for that because macros
     can reference macros that are later redefined. For example:

     #define A B
     int A;

     #define A C
     int A;.  */

  if (Already_Analyzed(info)) {
    return false;
  }

  AnalyzedMacros.insert(info);

  /* If this MacroInfo object with the macro contents wasn't marked for output
     then mark it now.  */
  if (!Is_Macro_Marked(info)) {
    MacroDependencies.insert(info);
    inserted = true;
  }

  /* We can not quickly return if the macro is already marked.  Other macros
     which this macro depends on may have been redefined in meanwhile, and
     we don't implement some dependency tracking so far, so redo the analysis.  */

  auto it = info->tokens_begin(); /* Use auto here as the it type changes according to clang version.  */
  /* Iterate on the expansion tokens of this macro to find if it references other
     macros.  */
  const Token *last_token = nullptr;
  for (; it != info->tokens_end(); ++it) {
    const Token *tok = it;
    const IdentifierInfo *tok_id = tok->getIdentifierInfo();
    tok::TokenKind tok_kind = tok->getKind();

    /* Handle the case where we concat symbols into a new one.  */
    if (tok_kind == tok::hashhash) {

      StringRef prevname = Get_Token_String(last_token);
      StringRef nextname = Get_Token_String(++it);

      if (MacroWalker::Is_Identifier_Macro_Argument(info, prevname)) {
        prevname = symtab[prevname.str()];
      }

      if (MacroWalker::Is_Identifier_Macro_Argument(info, nextname)) {
        nextname = symtab[nextname.str()];
      }

      Preprocessor &pprocessor = AST->getPreprocessor();
      std::string concat = prevname.str() + nextname.str();

      IdentifierInfo *concat_id = pprocessor.getIdentifierInfo(concat);
      MacroInfo *concat_macro = pprocessor.getMacroInfo(concat_id);
      if (concat_macro) {
        auto new_symtab = Get_Macro_Args_Replacement(concat_macro, it);
        inserted |= Backtrack_Macro_Expansion(concat_macro, loc, new_symtab);
      }

      it--;
    }

    if (tok_id != nullptr) {
      /* We must be careful to not confuse tokens which are function-like macro
         arguments with other macors.  Example:

         #define MAX(a, b) ((a) > (b) ? (a) : (b))
         #define a 9

         We should not add `a` as macro when analyzing MAX once it is clearly an
         argument of the macro, not a reference to other symbol.  */
      if (!MacroWalker::Is_Identifier_Macro_Argument(info, tok_id)) {
        MacroInfo *maybe_macro = MW.Get_Macro_Info(tok->getIdentifierInfo(), loc);

        /* If this token is actually a name to a declared macro, then analyze it
           as well.  */
        if (maybe_macro) {
          auto new_symtab = Get_Macro_Args_Replacement(maybe_macro, it);
          inserted |= Backtrack_Macro_Expansion(maybe_macro, loc, new_symtab);

        /* If the token is a name to a constant declared in an enum, then add
           the enum as well.  */
        } else if (EnumDecl *edecl = EnumTable.Get(tok->getIdentifierInfo()->getName())) {
          if (edecl && !Is_Decl_Marked(edecl)) {
            Add_Decl_And_Prevs(edecl);
          }
        }
      }
    }
    last_token = tok;
  }

  return inserted;
}

int MacroDependencyFinder::Populate_Need_Undef(void)
{
  int count = 0;
  PreprocessingRecord *rec = PProcessor.getPreprocessingRecord();

  for (PreprocessedEntity *entity : *rec) {
    if (MacroDefinitionRecord *def = dyn_cast<MacroDefinitionRecord>(entity)) {

      MacroDirective *directive = MW.Get_Macro_Directive(def);

      /* If there is no history, then doesn't bother analyzing.  */
      if (directive == nullptr)
        continue;

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

void MacroDependencyFinder::Remove_Redundant_Decls(void)
{
  ASTUnit::top_level_iterator it;

  for (it = AST->top_level_begin(); it != AST->top_level_end(); it++) {
    if (DeclaratorDecl *decl = dynamic_cast<DeclaratorDecl *>(*it)) {
      SourceRange range = decl->getSourceRange();

      const Type *type = decl->getType().getTypePtr();
      TagDecl *typedecl = type->getAsTagDecl();
      if (typedecl && Is_Decl_Marked(typedecl)) {
        SourceRange type_range = typedecl->getSourceRange();

        /* Using .fullyContains() fails in some declarations.  */
        if (PrettyPrint::Contains_From_LineCol(range, type_range)) {
          Dependencies.erase(typedecl);
        }
      }
    }
  }
}

void MacroDependencyFinder::Find_Macros_Required(void)
{
  /* If a SourceManager wasn't passed to the PrettyPrint class we cannot
     continue.  */
  assert(PrettyPrint::Get_Source_Manager() != nullptr);

  PreprocessingRecord *rec = PProcessor.getPreprocessingRecord();

  /* Ensure that PreprocessingRecord is avaialable.  */
  assert(rec && "PreprocessingRecord wasn't generated");

  clang::ASTUnit::top_level_iterator it = AST->top_level_begin();
  clang::PreprocessingRecord::iterator macro_it = rec->begin();

  while (it != AST->top_level_end()) {
    Decl *decl = *it;

    if (decl && Is_Decl_Marked(decl)) {
      SourceLocation expanded_decl_loc = PrettyPrint::Get_Expanded_Loc(decl);

      /* While the macro is before the end limit of the decl range, then:  */
      while (macro_it != rec->end() && PrettyPrint::Is_Before((*macro_it)->getSourceRange().getBegin(), expanded_decl_loc)) {
        SourceRange decl_range(decl->getSourceRange().getBegin(), expanded_decl_loc);

        /* If the macro location is in the Decl (function, variable, ...) range,
           then analyze this macro and its dependencies.  */
        if (PrettyPrint::Contains(decl_range, (*macro_it)->getSourceRange())) {
          if (MacroExpansion *macroexp = dyn_cast<MacroExpansion>(*macro_it)) {
            Backtrack_Macro_Expansion(macroexp);
          }
        }
        macro_it++;
      }
    }
    it++;
  }

  Populate_Need_Undef();
  Remove_Redundant_Decls();
}

void MacroDependencyFinder::Dump_Dependencies(void)
{
  std::unordered_set<MacroInfo*>::iterator itr;
  for (itr = MacroDependencies.begin(); itr != MacroDependencies.end(); itr++) {
    PrettyPrint::Print_MacroInfo(*itr);
    std::cout << '\n';
  }
}
