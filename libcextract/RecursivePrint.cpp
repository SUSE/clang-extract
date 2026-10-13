//===- RecursivePrint.cpp - Print Decls or Preprocessor entities to file *- C++ -*-===//
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

#include "RecursivePrint.hh"
#include "ClangCompat.hh"
#include "TopLevelASTIterator.hh"
#include "NonLLVMMisc.hh"
#include "LLVMMisc.hh"

#include <clang/AST/Attr.h>
#include <llvm/Support/Regex.h>

RecursivePrint::RecursivePrint(ASTUnit *ast,
                               std::unordered_set<Decl *> &deps,
                               IncludeTree &it,
                               raw_ostream &out,
                               bool keep_includes,
                               bool no_duplicated_includes)
  : AST(ast),
    SM(ast->getSourceManager()),
    LangOpts(ast->getLangOpts()),
    PPolicy(LangOpts),
    MW(ast->getPreprocessor()),
    Decl_Deps(deps),
    IT(it),
    Out(out),
    KeepIncludes(keep_includes)
{
  Analyze_Includes(no_duplicated_includes);
}

StringRef RecursivePrint::Get_Source_Text(const SourceRange &range)
{
  return ::Get_Source_Text(SM, LangOpts, range);
}

bool RecursivePrint::Is_Range_Valid(const SourceRange &range)
{
  const SourceLocation begin = range.getBegin();
  const SourceLocation end = range.getEnd();

  if (!begin.isValid() || !end.isValid())
    return false;

  return ::Is_Before(SM, begin, end) && (begin != end);
}

SourceLocation RecursivePrint::Get_Expanded_Loc(Decl *decl)
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

    bool has_attr = false;

    /* Vector type attributes are not encoded in the AttrVec structure, hence
       we have to check for its existence and expand until we match the ';'
       token.  See small/attr-11.c testcase.  */
    if (TypedefDecl *tdecl = dyn_cast<TypedefDecl>(decl)) {
      if (isa<clang::VectorType>(tdecl->getTypeSourceInfo()->getType().getTypePtr())) {
        has_attr = true;
      }
    }

    /* Get atttributes vector if it exists.  */
    if(decl->hasAttrs()) {
      AttrVec &attrvec = decl->getAttrs();
      for (size_t i = 0; i < attrvec.size(); i++) {
        const Attr *attr = attrvec[i];
        SourceLocation loc = attr->getRange().getEnd();
        loc = SM.getExpansionLoc(loc);

        if (loc.isValid() && ::Is_Before(SM, furthest, loc)) {
          furthest = loc;
          has_attr = true;
        }
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

        if (!maybe_next_tok.has_value()) {
          break;
        }

        const Token &tok = maybe_next_tok.value();
        head = tok.getLastLoc();

        /* Stop when we find the ';' token.  */
        if (tok.is(semicolon)) {
          /* Found the ';', now go back a character to *not* include the ';'.  */
          furthest = head.getLocWithOffset(-1);

          /* There is a bizarre case where a decl have an #endif at the end of
             it, just before the ';' (see small/attr-14.c).  Try to workaround
             this case by emiting the ';' that was there, which will result in
             two ';' being print.  This is visually not cute, but at least
             won't cause the compiler to cry.  */
          StringRef endstr = Get_Source_Text({furthest, furthest});
          if (const char *data_str = endstr.data()) {
            /* Check for newline.  */
            if (*data_str != '\0' && *data_str == '\n') {
              /* Set 'furthest' back to where it was.  */
              furthest = furthest.getLocWithOffset(1);
            }
          }
          break;
        }
      }
    }
  }

  /* In case this decl is a TypedefNameDecl, we also need to check for the
     struct definition for extra attributes.  See small/attr-10.c testscase
     for an example where this happens.  */
  if (TypedefNameDecl *typedecl = dyn_cast<TypedefNameDecl>(decl)) {
    if (TagDecl *tag = typedecl->getAnonDeclWithTypedefName()) {
      SourceLocation tag_furthest = Get_Expanded_Loc(tag);
      if (::Is_Before(SM, furthest, tag_furthest)) {
        furthest = tag_furthest;
      }
    }
  }

  return furthest;
}

void RecursivePrint::Print_MacroInfo(MacroInfo *info)
{
  SourceRange range(info->getDefinitionLoc(), info->getDefinitionEndLoc());
  Out << "#define " << Get_Source_Text(range) << '\n';
}

void RecursivePrint::Analyze_Includes(bool no_duplicated_includes)
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

  IT.Mark_Duplicated_Includes(no_duplicated_includes);

  /* Check for possible duplicated includes in the output.  */

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

void RecursivePrint::Print_CE_Comment(Decl *decl)
{
  ASTContext &ctx = AST->getASTContext();
  RawComment *comment = ClangCompat::getRawCommentNoCache(ctx, decl);
  Print_CE_Comment(decl->getBeginLoc(), comment);
}

void RecursivePrint::Print_CE_Comment([[maybe_unused]] MacroInfo *minfo)
{
  /* This feature is not supported in clang version < 23.  */
#if CLANG_VERSION_MAJOR >= 23
  ASTContext &ctx = AST->getASTContext();
  RawComment *comment = ClangCompat::getRawCommentNoCache(ctx, minfo);
  Print_CE_Comment(minfo->getDefinitionLoc(), comment);
#endif
}

void RecursivePrint::Print_CE_Comment(const SourceLocation &loc, RawComment *comment)
{
  if (loc.isValid()) {
    if (!Have_Location_Comment(SM, comment)) {
      Out << Build_CE_Location_Comment(SM, loc);
    } else {
      /* Just output what it had.  */
      Out << comment->getRawText(SM) << '\n';
    }
  }
}

void RecursivePrint::Print_Preprocessed(PreprocessedEntity *prep)
{
  if (MacroDefinitionRecord *entity = dyn_cast<MacroDefinitionRecord>(prep)) {
    MacroInfo *info = MW.Get_Macro_Info(entity);
    if (Is_Macro_Marked(info) && !MW.Is_Builtin_Macro(info)) {
      Print_CE_Comment(info);
      Out << "#define " << Get_Source_Text(entity->getSourceRange()) << "\n\n";
    }

    return;
  }

  if (KeepIncludes) {
    if (InclusionDirective *inc = dyn_cast<InclusionDirective>(prep)) {
      IncludeNode *node = IT.Get(inc);
      if (node && node->Should_Be_Output()) {
        Out << Get_Source_Text(inc->getSourceRange()) << '\n';
      }

      return;
    }
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
      bool print = true;
      if (KeepIncludes) {
        IncludeNode *node = IT.Get(undef_loc);
        print = node && node->Should_Be_Expanded();
      }

      if (print) {
        Out << "#undef " << Get_Source_Text(undef_loc) << '\n';
      }
    }
  }
}

/* Workarround LLVM bug #162126 (CE bug #146)*/
static void Attr_Order_Fix(Decl *decl)
{
  /* This only seems to affect variables, if its not the case then quickly
     return.  */
  VarDecl *vdecl = dyn_cast<VarDecl>(decl);
  if (vdecl == nullptr)
    return;

  if (decl->hasAttrs()) {
    AttrVec &Attrs = decl->getAttrs();

    /* Clang always seems to insert the `asm` label last, but lets just swap
       with the first one.  */
    for (unsigned i = 0; i < Attrs.size(); ++i) {
      if (isa<AsmLabelAttr>(Attrs[i])) {
        auto temp = Attrs[i];
        Attrs[i] = Attrs[0];
        Attrs[0] = temp;
        break;
      }
    }
  }
}

static bool Has_NoInline_Attr(Decl *decl)
{
  const AttrVec &attrs = decl->getAttrs();
  for (const Attr *attr : attrs) {
    if (attr->getKind() == attr::NoInline) {
      return true;
    }
  }

  return false;
}

void RecursivePrint::Print(void)
{
  Print_Decl_Context(AST->getASTContext().getTranslationUnitDecl());
}

void RecursivePrint::Print_Decl_Context(DeclContext *ctx)
{
  /* Iterate through the decls of ctx together with the macros defined in it,
     in the order they appear in the source code.  */
  TopLevelASTIterator it(AST, ctx, /*skip_macros_in_decl=*/false);

  while (!it.End()) {
    Decl *decl;

    switch ((*it).Type) {
      case TopLevelASTIterator::ReturnType::TYPE_INVALID:
        assert(0 && "Invalid type in TopLevelASTIterator.");
        break;

      case TopLevelASTIterator::ReturnType::TYPE_DECL:
        decl = (*it).AsDecl;
        Print_Decl(decl);
        ++it;
        /* Anything inside of the decl was already printed with it, e.g. the
           decls and macros of a namespace, or the source text of a function
           with its macros.  */
        it.Skip_Until(decl->getEndLoc());
        break;

      case TopLevelASTIterator::ReturnType::TYPE_PREPROCESSED_ENTITY:
        Print_Preprocessed((*it).AsPrep);
        ++it;
        break;

      case TopLevelASTIterator::ReturnType::TYPE_MACRO_UNDEF:
        Print_Macro_Undef((*it).AsUndef);
        ++it;
        break;
    }
  }
}

bool RecursivePrint::Is_Decl_Context_Needed(Decl *decl)
{
  if (Is_Decl_Marked(decl)) {
    return true;
  }

  /* The DeclContext is needed if any of the decls inside of it are.  */
  for (Decl *child : cast<DeclContext>(decl)->decls()) {
    if (isa<NamespaceDecl>(child) || isa<LinkageSpecDecl>(child)) {
      if (Is_Decl_Context_Needed(child)) {
        return true;
      }
    } else if (Is_Decl_Marked(child)) {
      return true;
    }
  }

  return false;
}

void RecursivePrint::Print_Namespace(NamespaceDecl *decl)
{

  /* Namespaces can contain many decls that are unused in the program, hence
     only print what is needed in it.  In case nothing is needed, there may
     still be macros defined in it.  */
  bool needed = Is_Decl_Context_Needed(decl);

  if (needed) {
    if (decl->isInline()) {
      Out << "inline ";
    }
    Out << "namespace " << decl->getName() << " {\n\n";
  }

  Print_Decl_Context(decl);

  if (needed) {
    Out << "}\n\n";
  }
}

void RecursivePrint::Print_Linkage_Spec(LinkageSpecDecl *decl)
{

  /* Same as namespaces, but for `extern "C" { ... }` blocks.  */
  bool needed = Is_Decl_Context_Needed(decl);

  /* `extern "C" int x;` is a declaration while `extern "C" { int x; }` is a
     definition, so do not add braces if there weren't any.  */
  bool braces = decl->hasBraces();

  if (needed) {
#if CLANG_VERSION_MAJOR >= 17
    bool is_c = decl->getLanguage() == LinkageSpecLanguageIDs::C;
#else
    bool is_c = decl->getLanguage() == LinkageSpecDecl::lang_c;
#endif
    Out << "extern \"" << (is_c ? "C" : "C++") << "\" ";
    if (braces) {
      Out << "{\n\n";
    }
  }

  Print_Decl_Context(decl);

  if (needed && braces) {
    Out << "}\n\n";
  }
}

void RecursivePrint::Print_Decl(Decl *decl)
{
  if (NamespaceDecl *namespacedecl = dyn_cast<NamespaceDecl>(decl)) {
    Print_Namespace(namespacedecl);
    return;
  }

  if (LinkageSpecDecl *linkagedecl = dyn_cast<LinkageSpecDecl>(decl)) {
    Print_Linkage_Spec(linkagedecl);
    return;
  }

  if (!Is_Decl_Marked(decl)) {
    return;
  }


  /* When handling C code, we need to check if given declaration is a function
     with body.  If yes, we can simply print the declaration, but otherwise
     we need to manually insert the end of statement ';' token.  */

  FunctionDecl *f = dyn_cast<FunctionDecl>(decl);
  TagDecl *t = dyn_cast<TagDecl>(decl);
  EnumDecl *e = dyn_cast<EnumDecl>(decl);

  Print_CE_Comment(decl);

  if (f && f->hasBody() && f->isThisDeclarationADefinition()) {
    Print_Decl_Raw(f);
    Out << "\n\n";
  } else if (!e && t && t->getName() == "") {
    /* If the RecordType doesn't have a name, then don't print it.  Except when
       it is an empty named enum declaration, which in this case we must print
       because it contains declared constants.  */
  } else {
    /* Structs, enums, and prototypes */

    /** Check if the output string has an include directive.  In that case, if we
        don't want to keep the includes, we must print the ast dump rather than
        the source text to get rid of the #include directive.
        FIXME:  regexes are slow.  */
    static llvm::Regex regex("# *include *(<|\")");

    /* FIXME: Why isCompleteDefinitionRequired does not work for EnumDecls?  */
    if (t && ((!e && t->isCompleteDefinitionRequired() == false)
              || (!KeepIncludes && regex.match(Get_Source_Text(t->getSourceRange()))))) {

      /* FIXME: The Print_Decl_Raw class will attempt to write this declaration
         as the user wrote, that means WITH a body.  To avoid this, we set
         the StartLocation to equal the End of location, which will trigger
         the "clang got confused" mechanism in Print_Decl_Raw and force it to
         be output as a tree dump instead of what the user wrote.  The
         correct way of doing this would be update the source location to
         the correct range.  */
      t->setLocStart(t->getEndLoc());
    }

    /** Check if we can get a partial declaration of `decl` rather than a full
        declaration.  */
    bool full_def_removed = false;
    /* FIXME: Why isCompleteDefinitionRequired does not work for EnumDecls?  */
    if (t && !e && t->isCompleteDefinitionRequired() == false) {
      /* We don't need the full defintion.  Hide the body for Print_Decl_Raw.  */
      t->setCompleteDefinition(false);
      full_def_removed = true;
    }

    Print_Decl_Raw(decl);
    Out << ';';
    if (full_def_removed) {
      Out << "/* Full definition was removed.  */";
    }
    Out << "\n\n";
  }
}

void RecursivePrint::Print_Decl_Raw(Decl *decl)
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

    SourceLocation furthest = Get_Expanded_Loc(decl);
    decl_range = SourceRange(decl_range.getBegin(), furthest);
    StringRef decl_source = Get_Source_Text(decl_range);

    /* If for some reason Get_Source_Text is unable to find the source range
       which comes this declaration because it is very complex, then fall back
       to AST dump.  */
    if (decl_source == "") {
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
          Attr_Order_Fix(decl);
          decl->print(Out, PPolicy);
        }
      } else {
        MacroInfo *noinline_info = nullptr;
        if (Has_NoInline_Attr(decl)) {
          /* If the code has a `noinline` macro defined, it conflicts with
             clang's __attribute__((noinline)) attribute dump.  Hence we
             have to undef it.  */
          Preprocessor &pp = AST->getPreprocessor();

          /* Get the valid macro at the source location.  */
          IdentifierInfo *noinline = pp.getIdentifierInfo("noinline");
          noinline_info = MW.Get_Macro_Info(noinline, decl->getBeginLoc());
          /* Only print it if the noinline macro is defined at location.  */
          if (noinline_info) {
            Out << "#undef noinline\n";
          }
        }
        Attr_Order_Fix(decl);
        decl->print(Out, PPolicy);
        if (noinline_info) {
          /* Redeclare the macro to the previous value.  */
          Print_MacroInfo(noinline_info);
        }
      }
    } else {
      /* Check if the output string has a balanced number of ifdefs.  This is
         required because of the following construct from glibc:

          #ifdef !SHARED
          #else
          struct rtld_global_ro {
          #endif

         In case we want to print rtld_global_ro, we will get the text
         from struct rtld_global_ro and below, which would only include the
         #endif, and not the #ifdef.  In case its not balanced we fallback to
         AST dumping.  */
      if (Has_Balanced_Ifdef(decl_source)) {
        Out << decl_source;
      } else {
        /* In case its not balanced, fallback to AST dump.  */
        Attr_Order_Fix(decl);
        decl->print(Out, PPolicy);
      }
    }
  } else {
    /* Else, we fallback to AST Dumping.  */
    Attr_Order_Fix(decl);
    decl->print(Out, PPolicy);
  }
}
