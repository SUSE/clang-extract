#include "PrettyPrint.hh"

/** Public methods.  */

void PrettyPrint::Print_Decl(Decl *decl)
{

  /* When handling C code, we need to check if given declaration is a function
     with body.  If yes, we can simply print the declaration, but otherwise
     we need to manually insert the end of statement ';' token.  */

  FunctionDecl *f = dynamic_cast<FunctionDecl*>(decl);
  TagDecl *t = dynamic_cast<TagDecl*>(decl);
  TypedefNameDecl *td = dynamic_cast<TypedefDecl*>(decl);

  if (f && f->hasBody() && f->isThisDeclarationADefinition()) {
    Print_Decl_Raw(f);
    Out << "\n";
  } else if (t && t->getName() == "") {
    /* If the RecordType doesn't have a name, then don't print it.  */
  } else if (td) {
    if (SM) {
    /* The Get_Source_Text will hang in the following case:
      struct A {
        int a;
      }

      typedef struct A A;

      It generates the following code:

      struct A {
        int a;
      }

      typedef struct A {
        int a;
      } A;

      which is a redefinition of struct A. Therefore we output the typedef manually.  */

      TagDecl *tagdecl = td->getAnonDeclWithTypedefName();

      if (tagdecl && tagdecl->getName() == "") {
        Print_Decl_Raw(decl);
        Out << ";\n";
      } else {
        decl->print(Out, PPolicy);
        Out << ";\n";
      }

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

void PrettyPrint::Print_Decl_Raw(Decl *decl)
{
  SourceRange decl_range = decl->getSourceRange();

  /* If a SourceManager was specified and the decl source range seems valid,
     then output based on the original source code.  */
  if (SM && Is_Before(decl_range.getBegin(), decl_range.getEnd())) {
    StringRef decl_source = Get_Source_Text(decl_range);
    Out << decl_source;
  } else {
    /* Else, we fallback to AST Dumping.  */
    decl->print(Out, LangOpts);
  }
}

void PrettyPrint::Print_Stmt(Stmt *stmt)
{
  /* Currently only used for debugging.  */
  stmt->printPretty(Out, nullptr, PPolicy);
  Out << "\n";
}

void PrettyPrint::Print_Macro_Def(MacroDefinitionRecord *rec)
{
  Out << "#define  " << Get_Source_Text(rec->getSourceRange()) << "\n";
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
  return is_before(a, b);
}

/** Compare if SourceLocation a is after SourceLocation b in the source code.  */
bool PrettyPrint::Is_After(const SourceLocation &a, const SourceLocation &b)
{
  assert(SM && "No SourceManager were given");
  BeforeThanCompare<SourceLocation> is_before(*SM);
  return is_before(b, a);
}



/* See PrettyPrint.hh for what they do.  */
raw_ostream &PrettyPrint::Out = llvm::outs();
LangOptions PrettyPrint::LangOpts;
PrintingPolicy PrettyPrint::PPolicy(LangOpts);
SourceManager *PrettyPrint::SM;
