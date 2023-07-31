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

void PrettyPrint::Print_Macro_Undef(MacroDirective *directive)
{
  SourceLocation loc = directive->getDefinition().getUndefLocation();
  assert(loc.isValid() && "Undefine location is invalid");
  Out << "#undef " << Get_Source_Text(loc) << '\n';
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

/* See PrettyPrint.hh for what they do.  */
raw_ostream *PrettyPrint::Out = &llvm::outs();
LangOptions PrettyPrint::LangOpts;
PrintingPolicy PrettyPrint::PPolicy(LangOpts);
SourceManager *PrettyPrint::SM;
