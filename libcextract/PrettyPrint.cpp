//===- PrettyPrint.cpp - Print Decls or Preprocessor entities to file *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Print Decls or Preprocessor entities to file.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#include "PrettyPrint.hh"
#include "ClangCompat.hh"
#include "TopLevelASTIterator.hh"
#include "NonLLVMMisc.hh"

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

  if (f && f->hasBody() && f->isThisDeclarationADefinition()) {
    Print_Decl_Raw(f);
    Out << "\n\n";
  } else if (e) {
      decl->print(Out, PPolicy);
      Out << ";\n\n";
  } else if (!e && t && t->getName() == "") {
    /* If the RecordType doesn't have a name, then don't print it.  Except when
       it is an empty named enum declaration, which in this case we must print
       because it contains declared constants.  */
  } else {
    /* Structs and prototypes */
    Print_Decl_Raw(decl);
    Out << ";\n\n";
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
  if (Is_Range_Valid(decl_range)) {

    /* The range given by getSourceRange ignores attributes. Example :

    int a __attribute__((aligned (8)));

    outputs:

    int a;

    Therefore we must check for the attributes of this declaration and compute
    the furthest location.  */

    SourceLocation furthest = PrettyPrint::Get_Expanded_Loc(decl);
    decl_range = SourceRange(decl_range.getBegin(), furthest);
    StringRef decl_source = Get_Source_Text(decl_range);

    /* If for some reason Get_Source_Text is unable to find the source range
       which comes this declaration because it is very complex, then fall back
       to AST dump.  */
    if (decl_source.equals("")) {
      /* TODO: warn user that we had to fallback to AST dump.  */

      if (TypedefNameDecl *typedecl = dyn_cast<TypedefNameDecl>(decl)) {
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
        TagDecl *tagdecl = typedecl->getAnonDeclWithTypedefName();
        if (tagdecl && tagdecl->getName() == "") {
          Out << "typedef ";
          tagdecl->getDefinition()->print(Out, PPolicy);
          Out << " " << typedecl->getName();
        } else {
          decl->print(Out, PPolicy);
        }
      } else {
        decl->print(Out, LangOpts);
      }
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
#undef Out
  auto o = Out;
  Out = &llvm::outs();
  Print_Decl(decl);;
  Out = o;
#define Out (*Out)
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

void PrettyPrint::Print_Comment(const std::string &comment)
{
  Out << "/** " << comment << "  */\n";
}

void PrettyPrint::Print_RawComment(SourceManager &sm, RawComment *comment)
{
  Out << comment->getRawText(sm) << '\n';
}

/** Private stuff.  */

StringRef PrettyPrint::Get_Source_Text(const SourceRange &range)
{
    // NOTE: sm.getSpellingLoc() used in case the range corresponds to a macro/preprocessed source.
    // NOTE2: getSpellingLoc() breaks in the case where a macro was asigned to be expanded to typedef.
    SourceManager &SM = AST->getSourceManager();
    auto start_loc = range.getBegin();//SM->getSpellingLoc(range.getBegin());
    auto last_token_loc = range.getEnd();//SM->getSpellingLoc(range.getEnd());
    auto end_loc = clang::Lexer::getLocForEndOfToken(last_token_loc, 0, SM, LangOpts);
    auto printable_range = clang::SourceRange{start_loc, end_loc};
    return Get_Source_Text_Raw(printable_range);
}

StringRef PrettyPrint::Get_Source_Text_Raw(const SourceRange &range)
{
    SourceManager &SM = AST->getSourceManager();
    return clang::Lexer::getSourceText(CharSourceRange::getCharRange(range), SM, LangOpts);
}

/** Compare if SourceLocation a is before SourceLocation b in the source code.  */
bool PrettyPrint::Is_Before(const SourceLocation &a, const SourceLocation &b)
{
  BeforeThanCompare<SourceLocation> is_before(AST->getSourceManager());

  assert(a.isValid());
  assert(b.isValid());

  return is_before(a, b);
}

void PrettyPrint::Debug_SourceLoc(const SourceLocation &loc)
{
  loc.dump(AST->getSourceManager());
}

bool PrettyPrint::Contains_From_LineCol(const SourceRange &a, const SourceRange &b)
{
  SourceManager &SM = AST->getSourceManager();
  PresumedLoc a_begin = SM.getPresumedLoc(a.getBegin());
  PresumedLoc a_end   = SM.getPresumedLoc(a.getEnd());
  PresumedLoc b_begin = SM.getPresumedLoc(b.getBegin());
  PresumedLoc b_end   = SM.getPresumedLoc(b.getEnd());

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
  BeforeThanCompare<SourceLocation> is_before(AST->getSourceManager());
  return is_before(b, a);
}

SourceLocation PrettyPrint::Get_Expanded_Loc(Decl *decl)
{
  SourceRange decl_range = decl->getSourceRange();
  SourceLocation furthest = decl_range.getEnd();

  /* If a SourceManager was specified and the decl source range seems valid,
     then output based on the original source code.  */
  if (Is_Range_Valid(decl_range)) {

    /* The range given by getSourceRange ignores attributes. Example :

    int a __attribute__((aligned (8)));

    outputs:

    int a;

    Therefore we must check for the attributes of this declaration and compute
    the furthest location.  */

    AttrVec &attrvec = decl->getAttrs();
    bool has_attr = false;
    SourceManager &SM = AST->getSourceManager();

    for (size_t i = 0; i < attrvec.size(); i++) {
      const Attr *attr = attrvec[i];
      SourceLocation loc = attr->getRange().getEnd();
      loc = SM.getExpansionLoc(loc);

      if (loc.isValid() && Is_Before(furthest, loc)) {
        furthest = loc;
        has_attr = true;
      }
    }

    /* Find the last ';' token, once there are attributes right after the last
       '}'.  On __attribute__((unused)), the previous code reach until
       __attribute__((unused), ignoring the last parenthesis. On macros, like
       __aligned(32), the code would skip (32).  */
    if (has_attr) {
      tok::TokenKind semicolon(tok::semi);
      SourceLocation head = furthest;

      /* Keep fetching tokens.  */
      while (true) {
        auto maybe_next_tok = Lexer::findNextToken(head, SM, LangOpts);
        Token *tok = ClangCompat_GetTokenPtr(maybe_next_tok);

        if (tok == nullptr) {
          break;
        }

        head = tok->getLastLoc();

        /* Stop when we find the ';' token.  */
        if (tok->is(semicolon)) {
          /* Found the ';', now go back a character to *not* include the ';'.  */
          furthest = head.getLocWithOffset(-1);
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
  return AST->getSourceManager().getFilename(loc);
}

OptionalFileEntryRef PrettyPrint::Get_FileEntry(const SourceLocation &loc)
{
  SourceManager &SM = AST->getSourceManager();
  return SM.getFileEntryRefForID(SM.getFileID(loc));
}

/* See PrettyPrint.hh for what they do.  */
raw_ostream *PrettyPrint::Out = &llvm::outs();
LangOptions PrettyPrint::LangOpts;
PrintingPolicy PrettyPrint::PPolicy(LangOpts);
ASTUnit *PrettyPrint::AST;



/** --- New RecursivePrint class code.  */

RecursivePrint::RecursivePrint(ASTUnit *ast,
                               std::unordered_set<Decl *> &deps,
                               IncludeTree &it,
                               bool keep_includes)
  : AST(ast),
    ASTIterator(ast, /*skip_macros_in_decl=*/false),
    MW(ast->getPreprocessor()),
    Decl_Deps(deps),
    IT(it),
    KeepIncludes(keep_includes)
{
  Analyze_Includes();
}

void RecursivePrint::Analyze_Includes(void)
{
  /* If we do not want to keep the includes then quickly return.  */
  if (KeepIncludes == false) {
    return;
  }

  MacroWalker mw(AST->getPreprocessor());

  /* Remove any macros that are provided by an include that should be output.  */
  PreprocessingRecord *rec = AST->getPreprocessor().getPreprocessingRecord();
  for (PreprocessedEntity *entity : *rec) {
    if (MacroDefinitionRecord *def = dyn_cast<MacroDefinitionRecord>(entity)) {
      SourceLocation loc = def->getLocation();
      if (IncludeNode *include = IT.Get(loc)) {
        if (include->Should_Be_Expanded() == false) {
          MacroDirective *macrodir = MW.Get_Macro_Directive(def);
          if (macrodir) {
            MacroInfo *macro = macrodir->getMacroInfo();
            macro->setIsUsed(false);
          }
        } else {
          /* In the case the header should be expanded, insert the macro from HeaderGuard.  */
          MacroDefinitionRecord *guard = include->Get_HeaderGuard();
          if (guard) {
            if (MacroInfo *guardinfo = mw.Get_Macro_Info(guard)) {
              guardinfo->setIsUsed(true);
            }
          }
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

void RecursivePrint::Print_Preprocessed(PreprocessedEntity *prep)
{
  if (MacroDefinitionRecord *entity = dyn_cast<MacroDefinitionRecord>(prep)) {
    MacroInfo *info = MW.Get_Macro_Info(entity);
    if (Is_Macro_Marked(info) && !MW.Is_Builtin_Macro(info)) {
      PrettyPrint::Print_Macro_Def(entity);
    }

    return;
  }

  if (KeepIncludes) {
    if (InclusionDirective *inc = dyn_cast<InclusionDirective>(prep)) {
      IncludeNode *node = IT.Get(inc);
      if (node && node->Should_Be_Output()) {
        PrettyPrint::Print_InclusionDirective(inc);
      }

      return;
    }
  }
}

static bool Have_Location_Comment(const SourceManager &sm, RawComment *comment)
{
  if (comment) {
    StringRef text = comment->getRawText(sm);
    if (prefix("/** clang-extract: from ", text.data())) {
      return true;
    }
  }
  return false;
}

void RecursivePrint::Print_Decl(Decl *decl)
{
  if (!Is_Decl_Marked(decl)) {
    return;
  }

  /* Handle namespaces.  Namespace declaration can contain many functions
     that can be unused in the program.  Hence we need to handle it
     carefully to remove what we don't need.  */
  if (NamespaceDecl *namespacedecl = dynamic_cast<NamespaceDecl*>(decl)) {
    if (namespacedecl->isInline()) {
       (*PrettyPrint::Out)  << "inline ";
    }

    (*PrettyPrint::Out) <<"namespace " << namespacedecl->getName() << " {\n  ";

    /* Iterate on each macro.  */
    for (auto child : namespacedecl->decls()) {
      Print_Decl(child);
    }
    (*PrettyPrint::Out) << "}\n";
  } else {

    SourceManager &sm = AST->getSourceManager();
    ASTContext &ctx = AST->getASTContext();
    RawComment *comment = ctx.getRawCommentForDeclNoCache(decl);
    if (decl->getBeginLoc().isValid()) {
      if (!Have_Location_Comment(sm, comment)) {
        PresumedLoc presumed = sm.getPresumedLoc(decl->getBeginLoc());
        unsigned line = presumed.getLine();
        unsigned col = presumed.getColumn();
        const std::string &filename = sm.getFilename(decl->getBeginLoc()).str();

        std::string comment = "clang-extract: from " + filename + ":" +
                              std::to_string(line) + ":" + std::to_string(col);
        PrettyPrint::Print_Comment(comment);
      } else {
        /* Just output what it had.  */
        PrettyPrint::Print_RawComment(sm, comment);
      }
    }
    PrettyPrint::Print_Decl(decl);
  }
}

void RecursivePrint::Print_Macro_Undef(MacroDirective *directive)
{
  const SourceLocation &undef_loc = directive->getDefinition().getUndefLocation();

  /* In case the macro isn't undefined then its location is invalid.  */
  if (undef_loc.isValid()) {
    /* Check if the macro is marked for output.  */
    MacroInfo *info = directive->getMacroInfo();
    if (Is_Macro_Marked(info)) {
      /* Check if the undef location actually points to an expanded include
         or the main file.  */
      if (KeepIncludes) {
        IncludeNode *node = IT.Get(undef_loc);
        if (KeepIncludes && node) {
          if (node->Should_Be_Expanded()) {
            PrettyPrint::Print_Macro_Undef(directive);
          }
        }
      } else {
        PrettyPrint::Print_Macro_Undef(directive);
      }
    }
  }
}

void RecursivePrint::Print(void)
{
  while (!ASTIterator.End()) {
    Decl *decl;

    switch ((*ASTIterator).Type) {
      case TopLevelASTIterator::ReturnType::TYPE_INVALID:
        assert(0 && "Invalid type in ASTIterator.");
        break;

      case TopLevelASTIterator::ReturnType::TYPE_DECL:
        decl = (*ASTIterator).AsDecl;
        Print_Decl(decl);
        ++ASTIterator;

        /* Skip to the end of the Declaration.  */
        if (!dyn_cast<EnumDecl>(decl)) {
          /* EnumDecls are handled somewhat differently: we dump the AST, not
             what the user wrote.  */
          ASTIterator.Skip_Until(decl->getEndLoc());
        }
        break;

      case TopLevelASTIterator::ReturnType::TYPE_PREPROCESSED_ENTITY:
        Print_Preprocessed((*ASTIterator).AsPrep);
        ++ASTIterator;
        break;

      case TopLevelASTIterator::ReturnType::TYPE_MACRO_UNDEF:
        Print_Macro_Undef((*ASTIterator).AsUndef);
        ++ASTIterator;
        break;
    }
  }
}
