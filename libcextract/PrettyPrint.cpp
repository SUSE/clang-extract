#include "PrettyPrint.hh"
#include "ClangCompat.hh"

#include <clang/AST/Attr.h>

/** Public methods.  */

#define Out (*Out)

void PrettyPrint::Print_Decl(Decl *decl)
{
  /* When handling C code, we need to check if given declaration is a function
     with body.  If yes, we can simply print the declaration, but otherwise
     we need to manually insert the end of statement ';' token.  */

  FunctionDecl *f = dynamic_cast<FunctionDecl*>(decl);
  TagDecl *t = dynamic_cast<TagDecl*>(decl);
  EnumDecl *e = dynamic_cast<EnumDecl*>(decl);
  TypedefNameDecl *td = dynamic_cast<TypedefDecl*>(decl);

  if (f && f->hasBody() && f->isThisDeclarationADefinition()) {
    Print_Decl_Raw(f);
    Out << "\n";
  } else if (e) {
      decl->print(Out, PPolicy);
      Out << ";\n";
  } else if (!e && t && t->getName() == "") {
    /* If the RecordType doesn't have a name, then don't print it.  Except when
       it is an empty named enum declaration, which in this case we must print
       because it contains declared constants.  */
  } else if (td) {
    if (SM) {
      Print_Decl_Raw(decl);
      Out << ";\n";
    } else {

    /* The AST dump will hang on the following case:

       typedef struct {
         attrs
       } Type;

       That is because clang decompose such constructions in

       struct {
         attrs
       };

       typedef struct Type Type;

       but since the struct has no name, Type goes undefined.

       The way we fix that is output the typedef and the TagDecl
       body, and we avoid outputs of unamed structs.

       This problem does not seem to show up if we are printing
       from source location.  */

      TagDecl *tagdecl = td->getAnonDeclWithTypedefName();

      if (tagdecl && tagdecl->getName() == "") {
        Out << "typedef ";
        tagdecl->getDefinition()->print(Out, PPolicy);
        Out << " " << td->getName() << ";\n";
      } else {
        decl->print(Out, PPolicy);
        Out << ";\n";
      }
    }

  } else {
    Print_Decl_Raw(decl);
    Out << ";\n";
  }
}

bool PrettyPrint::Is_Range_Valid(const SourceRange &range)
{
  const SourceLocation begin = range.getBegin();
  const SourceLocation end = range.getEnd();

  if (!begin.isValid() || !end.isValid())
    return false;

  return Is_Before(begin, end) && (begin != end);
}

void PrettyPrint::Print_Decl_Raw(Decl *decl)
{
  SourceRange decl_range = decl->getSourceRange();

  /* If a SourceManager was specified and the decl source range seems valid,
     then output based on the original source code.  */
  if (SM && Is_Range_Valid(decl_range)) {

    /* The range given by getSourceRange ignores attributes. Example :

    int a __attribute__((aligned (8)));

    outputs:

    int a;

    Therefore we must check for the attributes of this declaration and compute
    the furthest location.  */

    SourceLocation furthest = decl_range.getEnd();
    AttrVec &attrvec = decl->getAttrs();
    bool has_attr;

    for (size_t i = 0; i < attrvec.size(); i++) {
      SourceLocation loc = attrvec[i]->getRange().getEnd();
      loc = Lexer::getLocForEndOfToken(loc, 0, *SM, LangOpts);

      if (loc.isValid() && Is_Before(furthest, loc)) {
        furthest = loc;
        has_attr = true;
      }
    }

    /* Find the last ')'.  On __attribute__((unused)), the previous code reach
       until __attribute__((unused), ignoring the last parenthesis.  */
    if (has_attr) {

      tok::TokenKind r_paren(tok::r_paren);

      while (true) {
        auto maybe_next_tok = Lexer::findNextToken(furthest, *SM, LangOpts);
        Token *tok = ClangCompat_GetTokenPtr(maybe_next_tok);

        if (tok == nullptr) {
          break;
        }

        if (tok->is(r_paren)) {
          furthest = tok->getLastLoc();
        } else {
          break;
        }
      }
    }

    decl_range = SourceRange(decl_range.getBegin(), furthest);
    StringRef decl_source = Get_Source_Text(decl_range);

    /* If for some reason Get_Source_Text is unable to find the source range
       which comes this declaration because it is very complex, then fall back
       to AST dump.  */
    if (decl_source.equals("")) {
      /* TODO: warn user that we had to fallback to AST dump.  */
      decl->print(Out, LangOpts);
    } else {
      Out << decl_source;
    }
  } else {
    /* Else, we fallback to AST Dumping.  */
    decl->print(Out, LangOpts);
  }
}

void PrettyPrint::Debug_Decl(Decl *decl)
{
  llvm::outs() << Get_Source_Text(decl->getSourceRange()) << '\n';
}

void PrettyPrint::Debug_Stmt(Stmt *stmt)
{
  stmt->printPretty(llvm::outs(), nullptr, PPolicy);
}

void PrettyPrint::Print_Stmt(Stmt *stmt)
{
  /* Currently only used for debugging.  */
  stmt->printPretty(Out, nullptr, PPolicy);
  Out << "\n";
}

void PrettyPrint::Print_Macro_Def(MacroDefinitionRecord *rec)
{
  Out << "#define " << Get_Source_Text(rec->getSourceRange()) << "\n";
}

void PrettyPrint::Debug_Macro_Def(MacroDefinitionRecord *rec)
{
  llvm::outs() << "#define " << Get_Source_Text(rec->getSourceRange()) << "\n";
}

void PrettyPrint::Print_Macro_Undef(MacroDirective *directive)
{
  SourceLocation loc = directive->getDefinition().getUndefLocation();
  assert(loc.isValid() && "Undefine location is invalid");
  Out << "#undef " << Get_Source_Text(loc) << '\n';
}

void PrettyPrint::Debug_Macro_Undef(MacroDirective *directive)
{
  SourceLocation loc = directive->getDefinition().getUndefLocation();
  assert(loc.isValid() && "Undefine location is invalid");
  llvm::outs() << "#undef " << Get_Source_Text(loc) << '\n';
}

void PrettyPrint::Print_InclusionDirective(InclusionDirective *include)
{
  Out << Get_Source_Text(include->getSourceRange()) << '\n';
}

void PrettyPrint::Debug_InclusionDirective(InclusionDirective *include)
{
  llvm::outs() << Get_Source_Text(include->getSourceRange()) << '\n';
}

void PrettyPrint::Print_MacroInfo(MacroInfo *info)
{
  SourceRange range(info->getDefinitionLoc(), info->getDefinitionEndLoc());
  llvm::outs() << " info: " << Get_Source_Text(range) << '\n';
}

void PrettyPrint::Print_Attr(Attr *attr)
{
  attr->printPretty(llvm::outs(), PPolicy);
  llvm::outs() << '\n';
}

/** Private stuff.  */

StringRef PrettyPrint::Get_Source_Text(const SourceRange &range)
{
    /* Calling this function supposes that a SourceManager was given to this class.  */
    assert(SM && "A SourceManager wasn't passed to PrettyPrint.");

    // NOTE: sm.getSpellingLoc() used in case the range corresponds to a macro/preprocessed source.
    // NOTE2: getSpellingLoc() breaks in the case where a macro was asigned to be expanded to typedef.
    auto start_loc = range.getBegin();//SM->getSpellingLoc(range.getBegin());
    auto last_token_loc = range.getEnd();//SM->getSpellingLoc(range.getEnd());
    auto end_loc = clang::Lexer::getLocForEndOfToken(last_token_loc, 0, *SM, LangOpts);
    auto printable_range = clang::SourceRange{start_loc, end_loc};
    return Get_Source_Text_Raw(printable_range);
}

StringRef PrettyPrint::Get_Source_Text_Raw(const SourceRange &range)
{
    return clang::Lexer::getSourceText(CharSourceRange::getCharRange(range), *SM, LangOpts);
}

/** Compare if SourceLocation a is before SourceLocation b in the source code.  */
bool PrettyPrint::Is_Before(const SourceLocation &a, const SourceLocation &b)
{
  assert(SM && "No SourceManager were given");
  BeforeThanCompare<SourceLocation> is_before(*SM);

  assert(a.isValid());
  assert(b.isValid());

  return is_before(a, b);
}

void PrettyPrint::Debug_SourceLoc(const SourceLocation &loc)
{
  loc.dump(*SM);
}

bool PrettyPrint::Contains_From_LineCol(const SourceRange &a, const SourceRange &b)
{
  PresumedLoc a_begin = SM->getPresumedLoc(a.getBegin());
  PresumedLoc a_end   = SM->getPresumedLoc(a.getEnd());
  PresumedLoc b_begin = SM->getPresumedLoc(b.getBegin());
  PresumedLoc b_end   = SM->getPresumedLoc(b.getEnd());

  assert(a_begin.getFileID() == a_end.getFileID());
  assert(b_begin.getFileID() == b_end.getFileID());

  if (a_begin.getFileID() != b_begin.getFileID()) {
    /* Files are distinct, thus we can't easily determine which comes first.  */
    return false;
  }

  bool a_begin_smaller = false;
  bool b_end_smaller = false;

  if ((a_begin.getLine() < b_begin.getLine()) ||
      (a_begin.getLine() == b_begin.getLine() && a_begin.getColumn() <= b_begin.getColumn())) {
    a_begin_smaller = true;
  }

  if ((b_end.getLine() < a_end.getLine()) ||
      (b_end.getLine() == a_end.getLine() && b_end.getColumn() <= a_end.getColumn())) {
    b_end_smaller = true;
  }

  return a_begin_smaller && b_end_smaller;
}

bool PrettyPrint::Contains(const SourceRange &a, const SourceRange &b)
{
  if (a.fullyContains(b)) {
    return true;
  }

  return Contains_From_LineCol(a, b);
}

/** Compare if SourceLocation a is after SourceLocation b in the source code.  */
bool PrettyPrint::Is_After(const SourceLocation &a, const SourceLocation &b)
{
  assert(SM && "No SourceManager were given");
  BeforeThanCompare<SourceLocation> is_before(*SM);
  return is_before(b, a);
}

SourceLocation PrettyPrint::Get_Expanded_Loc(Decl *decl)
{
  SourceRange decl_range = decl->getSourceRange();
  SourceLocation furthest = decl_range.getEnd();

  /* If a SourceManager was specified and the decl source range seems valid,
     then output based on the original source code.  */
  if (SM && Is_Range_Valid(decl_range)) {

    /* The range given by getSourceRange ignores attributes. Example :

    int a __attribute__((aligned (8)));

    outputs:

    int a;

    Therefore we must check for the attributes of this declaration and compute
    the furthest location.  */

    AttrVec &attrvec = decl->getAttrs();
    bool has_attr;

    for (size_t i = 0; i < attrvec.size(); i++) {
      SourceLocation loc = attrvec[i]->getRange().getEnd();
      loc = Lexer::getLocForEndOfToken(loc, 0, *SM, LangOpts);

      if (loc.isValid() && Is_Before(furthest, loc)) {
        furthest = loc;
        has_attr = true;
      }
    }

    /* Find the last ')'.  On __attribute__((unused)), the previous code reach
       until __attribute__((unused), ignoring the last parenthesis.  */
    if (has_attr) {

      tok::TokenKind r_paren(tok::r_paren);

      while (true) {
        auto maybe_next_tok = Lexer::findNextToken(furthest, *SM, LangOpts);
        Token *tok = ClangCompat_GetTokenPtr(maybe_next_tok);

        if (tok == nullptr) {
          break;
        }

        if (tok->is(r_paren)) {
          furthest = tok->getLastLoc();
        } else {
          break;
        }
      }
    }
  }
  return furthest;
}

#undef Out

/** Set output to file.  */
void PrettyPrint::Set_Output_To(const std::string &path)
{
  std::error_code ec;
  static llvm::raw_fd_ostream out(path, ec);

  Set_Output_Ostream(&out);
}

StringRef PrettyPrint::Get_Filename_From_Loc(const SourceLocation &loc)
{
  return SM->getFilename(loc);
}

OptionalFileEntryRef PrettyPrint::Get_FileEntry(const SourceLocation &loc)
{
  return SM->getFileEntryRefForID(SM->getFileID(loc));
}

/* See PrettyPrint.hh for what they do.  */
raw_ostream *PrettyPrint::Out = &llvm::outs();
LangOptions PrettyPrint::LangOpts;
PrintingPolicy PrettyPrint::PPolicy(LangOpts);
SourceManager *PrettyPrint::SM;



/** --- New RecursivePrint class code.  */

RecursivePrint::RecursivePrint(ASTUnit *ast,
                               std::unordered_set<Decl *> &deps,
                               IncludeTree &it,
                               bool keep_includes)
  : AST(ast),
    MW(ast->getPreprocessor()),
    Decl_Deps(deps),
    IT(it),
    KeepIncludes(keep_includes)
{
  MI.macro_it = AST->getPreprocessor().getPreprocessingRecord()->begin();
  MI.undef_it = 0;

  Analyze_Includes();
  Populate_Need_Undef();
}

void RecursivePrint::Analyze_Includes(void)
{
  /* If we do not want to keep the includes then quickly return.  */
  if (KeepIncludes == false) {
    return;
  }

  /* Remove any macros that are provided by an include that should be output.  */
  PreprocessingRecord *rec = AST->getPreprocessor().getPreprocessingRecord();
  for (PreprocessedEntity *entity : *rec) {
    if (MacroDefinitionRecord *def = dyn_cast<MacroDefinitionRecord>(entity)) {
      SourceLocation loc = def->getLocation();
      IncludeNode *include = IT.Get(loc);

      if (include != nullptr && include->Should_Be_Expanded() == false) {
        MacroDirective *macrodir = MW.Get_Macro_Directive(def);
        if (macrodir) {
          MacroInfo *macro = macrodir->getMacroInfo();
          macro->setIsUsed(false);
        }
      }
    }
  }

  /* Remove any decls that are provided by an include that should be output.  */
  for (auto it = Decl_Deps.begin(); it != Decl_Deps.end();) {
    SourceLocation loc = (*it)->getLocation();
    IncludeNode *include = IT.Get(loc);

    /* The location is covered by a include.  Now check if this include is not
       marked for expansion.  */
    if (include != nullptr && include->Should_Be_Expanded() == false) {
      /* If not we can safely remove this decl.  */
      it = Decl_Deps.erase(it);
    } else {
      it++;
    }
  }
}

void RecursivePrint::Print(void)
{
  /** Print the root node "Translation Unit".  */
  Print_Decl(AST->getASTContext().getTranslationUnitDecl());
}

void RecursivePrint::Print_Decl(Decl *decl)
{
  if (TranslationUnitDecl *tu = dynamic_cast<TranslationUnitDecl*>(decl)) {
    for (Decl *d : tu->decls()) {
      Print_Preprocessor_Until(d->getBeginLoc());
      Print_Decl(d);
    }

    /* Print remaining macros located after all declarations.  */
    SourceManager &sm = AST->getSourceManager();
    SourceLocation end = sm.getLocForEndOfFile(sm.getMainFileID());

    Print_Preprocessor_Until(end);
  } else if (Is_Decl_Marked(decl)) {
    /* Handle namespaces.  Namespace declaration can contain many functions
       that can be unused in the program.  Hence we need to handle it
       carefully to remove what we don't need.  */
    if (NamespaceDecl *namespacedecl = dynamic_cast<NamespaceDecl*>(decl)) {
      (*PrettyPrint::Out) <<"namespace " << namespacedecl->getName() << " {\n  ";

      /* Iterate on each macro.  */
      for (auto child : namespacedecl->decls()) {
        Print_Decl(child);
      }
      (*PrettyPrint::Out) << "}\n";
    } else {
      SourceLocation l1 = decl->getBeginLoc();
      Skip_Preprocessor_Until(l1);
      PrettyPrint::Print_Decl(decl);
    }
  }
}

/** Sort function for Macros.  */
static bool Compare_Macro_Undef_Loc(MacroDirective *a, MacroDirective *b)
{
  return PrettyPrint::Is_Before(a->getDefinition().getUndefLocation(),
                                b->getDefinition().getUndefLocation());
}

void RecursivePrint::Populate_Need_Undef(void)
{
  PreprocessingRecord *rec = AST->getPreprocessor().getPreprocessingRecord();

  for (PreprocessedEntity *entity : *rec) {
    if (MacroDefinitionRecord *def = dyn_cast<MacroDefinitionRecord>(entity)) {

      MacroDirective *directive = MW.Get_Macro_Directive(def);

      /* If there is no history, then doesn't bother analyzing.  */
      if (directive == nullptr)
        continue;

      /* There is no point in analyzing a macro that wasn't added for output. */
      if (Is_Macro_Marked(directive->getMacroInfo())) {
        SourceLocation undef_loc =
            directive->getDefinition().getUndefLocation();

        /* In case the macro isn't undefined then its location is invalid.  */
        if (undef_loc.isValid()) {
          NeedsUndef.push_back(directive);
        }
      }
    }
  }

  std::sort(NeedsUndef.begin(), NeedsUndef.end(), Compare_Macro_Undef_Loc);
}

void RecursivePrint::Print_Preprocessor_Until(const SourceLocation &loc, bool print)
{
  PreprocessingRecord *rec = AST->getPreprocessor().getPreprocessingRecord();
  SourceManager *sm = &AST->getSourceManager();

  SourceLocation curr_loc;

  /* We have to iterate in both the preprocessor record and the marked undef
     vector to find which one we should print based on the location.  This is
     somewhat similar to how mergesort merge two sorted vectors, if this
     somehow helps to understand this code.  */
  while (true) {
    PreprocessedEntity *e = nullptr;
    MacroDirective *undef = nullptr;

    /* Initialize with the last possible location for the case we already
       covered all #defines or all #undefs.  */
    SourceLocation define_loc = sm->getLocForEndOfFile(sm->getMainFileID());
    SourceLocation undef_loc = define_loc;

    /* Initialize variables.  */
    if (MI.macro_it != rec->end()) {
      e = *MI.macro_it;
      define_loc = e->getSourceRange().getBegin();
    }
    if (MI.undef_it < NeedsUndef.size()) {
      undef = NeedsUndef[MI.undef_it];
      undef_loc = undef->getDefinition().getUndefLocation();
    }

    /* If we can't get any macros or undefs, then return because we are done.  */
    if (e == nullptr && undef == nullptr) {
      return;
    }

    /* Find out which comes first.  */
    if (PrettyPrint::Is_Before(define_loc, undef_loc)) {
      curr_loc = define_loc;
      /* Mark undefine as invalid.  */
      undef = nullptr;
    } else {
      curr_loc = undef_loc;
      /* Mark define as invalid.  */
      e = nullptr;
    }

    /* If we passed the mark to where we should stop printing, then we are done.  */
    if (PrettyPrint::Is_After(curr_loc, loc)) {
      return;
    }

    /* Print wathever comes first.  */
    if (e) {
      if (MacroDefinitionRecord *entity = dyn_cast<MacroDefinitionRecord>(e)) {
        if (print) {
          MacroInfo *info = MW.Get_Macro_Info(entity);
          if (Is_Macro_Marked(info) && !MW.Is_Builtin_Macro(info)) {
            PrettyPrint::Print_Macro_Def(entity);
          }
        }
      } else if (KeepIncludes) {
        if (InclusionDirective *inc = dyn_cast<InclusionDirective>(e)) {
          IncludeNode *node = IT.Get(inc);
          if (node && node->Should_Be_Output()) {
            PrettyPrint::Print_InclusionDirective(inc);
          }
        }
      }
      MI.macro_it++;
    } else if (undef) {
      if (print) {
        PrettyPrint::Print_Macro_Undef(undef);
      }
      MI.undef_it++;
    }
  }
}
