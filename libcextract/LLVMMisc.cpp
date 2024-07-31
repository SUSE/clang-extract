//===- LLVMMisc.cpp - Implement misc functions that depends on LLVM *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// MISC functions that is used through the program and use any LLVM
/// datastructure.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#include "LLVMMisc.hh"
#include "NonLLVMMisc.hh"

/** Check if Decl is a builtin.  */
bool Is_Builtin_Decl(const Decl *decl)
{
  const NamedDecl *ndecl = dyn_cast<const NamedDecl>(decl);
  if (ndecl) {
    /* Avoid triggering an assert in clang.  */
    if (ndecl->getDeclName().isIdentifier()) {
      StringRef name = ndecl->getName();
      if (name.starts_with("__builtin_")) {
        return true;
      }

      if (name.starts_with("__compiletime_assert_")) {
        return true;
      }
    }
  }

  return false;
}

/** Build CallGraph from AST.
 *
 * The CallGraph is a datastructure in which nodes are functions and edges
 * represents function call points. For example:
 *
 * void f();
 * void g() { f(); f(); }
 *
 * Resuluts in the following CallGraph:
 *
 * (f) -> (g)
 *     -> (g)
 *
 * There are two edges to `g` because there are two callpoints to it.
 * Hence, the resulting graph is not `simple` in Graph Theory nomenclature. But
 * for the analysis we are doing it is suffice to be, so perhaps some extra
 * performance can be archived if we could remove duplicated edges.
 *
 */
CallGraph *Build_CallGraph_From_AST(ASTUnit *ast)
{
  CallGraph *cg = new CallGraph();
  cg->TraverseAST(ast->getASTContext());

  return cg;
}


FunctionDecl *Get_Bodyless_Decl(FunctionDecl *decl)
{
  while (decl) {
    if (decl->hasBody()) {
      decl = decl->getPreviousDecl();
    } else {
      return decl;
    }
  }

  return nullptr;
}

FunctionDecl *Get_Bodyless_Or_Itself(FunctionDecl *decl)
{
  FunctionDecl *bodyless = Get_Bodyless_Decl(decl);
  return bodyless ? bodyless : decl;
}

TagDecl *Get_Bodyless_Decl(TagDecl *decl)
{
  while (decl) {
    if (decl->isCompleteDefinition()) {
      decl = decl->getPreviousDecl();
    } else {
      return decl;
    }
  }

  return nullptr;
}

TagDecl *Get_Bodyless_Or_Itself(TagDecl *decl)
{
  TagDecl *bodyless = Get_Bodyless_Decl(decl);
  return bodyless ? bodyless : decl;
}

Decl *Get_Bodyless_Decl(Decl *decl)
{
  if (TagDecl *tag = dyn_cast<TagDecl>(decl)) {
    return Get_Bodyless_Decl(tag);
  } else if (FunctionDecl *func = dyn_cast<FunctionDecl>(decl)) {
    return Get_Bodyless_Decl(func);
  }

  return nullptr;
}

Decl *Get_Bodyless_Or_Itself(Decl *decl)
{
  Decl *bodyless = Get_Bodyless_Decl(decl);
  return bodyless ? bodyless : decl;
}

/* Get the TopLevel Decl that contains the location loc.  */
Decl *Get_Toplevel_Decl_At_Location(ASTUnit *ast, const SourceLocation &loc)
{
  SourceManager &SM = ast->getSourceManager();
  /* We don't have a way of accessing the TopLevel vector directly, hence we
     do this.  */
  char *p = (char *) &(*ast->top_level_begin());
  char *q = (char *) &(*ast->top_level_end());

  int n = (((ptrdiff_t)(q - p))/sizeof(Decl *));

  Decl **array = (Decl **)p;

  /* Do binary search.  */
  int low = 0;
  int high = n-1;
  while (low <= high) {
    int mid = low + (high - low)/2;

    Decl *decl = array[mid];
    /* Get rid of some weird macro locations.  We want the location where
       it was expanded.  */
    SourceRange decl_range(SM.getExpansionLoc(decl->getBeginLoc()),
                           SM.getExpansionLoc(decl->getEndLoc()));

    if (decl_range.fullyContains(loc)) {
      return decl;
    }

    if (SM.isBeforeInTranslationUnit(decl_range.getBegin(), loc)) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }

  return nullptr;
}

VectorRef<Decl *> Get_Toplev_Decls_With_Same_Beginloc(ASTUnit *ast, const SourceLocation &loc)
{
  SourceManager &SM = ast->getSourceManager();
  /* We don't have a way of accessing the TopLevel vector directly, hence we
     do this.  */
  char *p = (char *) &(*ast->top_level_begin());
  char *q = (char *) &(*ast->top_level_end());

  int n = (((ptrdiff_t)(q - p))/sizeof(Decl *));

  Decl **array = (Decl **)p;

  /* Do binary search.  */
  int low = 0;
  int high = n-1;
  while (low <= high) {
    int mid = low + (high - low)/2;

    Decl *decl = array[mid];
    /* Get rid of some weird macro locations.  We want the location where
       it was expanded.  */
    SourceLocation begin = SM.getExpansionLoc(decl->getBeginLoc());

    if (begin == loc) {
      /* Go back to the first declaration.  */
      int last_l = mid;
      for (int l = mid; l >= 0; --l) {
        if (SM.getExpansionLoc(array[l]->getBeginLoc()) == loc) {
          last_l = l;
        } else {
          break;
        }
      }

      /* Go forward to the last declaration.  */
      int last_h = mid;
      for (int h = mid; h < n; ++h) {
        if (SM.getExpansionLoc(array[h]->getBeginLoc()) == loc) {
          last_h = h;
        } else {
          break;
        }
      }

      return VectorRef(&array[last_l], &array[last_h]);
    }

    if (SM.isBeforeInTranslationUnit(begin, loc)) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }

  return VectorRef<Decl *>(nullptr, 0U);
}

std::string Build_CE_Location_Comment(SourceManager &sm, const SourceLocation &loc)
{
  PresumedLoc presumed = sm.getPresumedLoc(loc);
  unsigned line = presumed.getLine();
  unsigned col = presumed.getColumn();
  const char *filename = presumed.getFilename();

  std::string comment = "/** clang-extract: from " + std::string(filename) + ":" +
                       std::to_string(line) + ":" + std::to_string(col) + " */\n";
  return comment;
}

/** Get decl clang-extract location comment, or build one if it doesn't exist.  */
std::string Get_Or_Build_CE_Location_Comment(ASTContext &ctx, Decl *decl)
{
  SourceManager &sm = ctx.getSourceManager();
  RawComment *comment = ctx.getRawCommentForDeclNoCache(decl);
  if (Have_Location_Comment(sm, comment)) {
    return comment->getRawText(sm).str();;
  } else {
    return Build_CE_Location_Comment(sm, decl->getBeginLoc());
  }
}

/** Get the begin location of the Decl before its comment if it have one.  */
SourceLocation Get_Begin_Loc_Of_Decl_Or_Comment(ASTContext &ctx, Decl *decl)
{
  if (RawComment *comment = ctx.getRawCommentForDeclNoCache(decl)) {
    return comment->getBeginLoc();
  } else {
    return decl->getBeginLoc();
  }
}

/** Check if Decl have a Location comment.  */
bool Have_Location_Comment(const SourceManager &sm, RawComment *comment)
{
  if (comment) {
    StringRef text = comment->getRawText(sm);
    if (prefix("/** clang-extract: from ", text.data())) {
      return true;
    }
  }
  return false;
}

/** Lookup in the symbol table for a declaration with given name passed by info.  */
DeclContextLookupResult Get_Decl_From_Symtab(ASTUnit *ast, const IdentifierInfo *info)
{
  TranslationUnitDecl *tu = ast->getASTContext().getTranslationUnitDecl();
  return tu->lookup(DeclarationName(info));
}

/** Lookup in the symbol table for a declaration with given name passed by name.  */
DeclContextLookupResult Get_Decl_From_Symtab(ASTUnit *ast, const StringRef &name)
{
  IdentifierTable &symtab = ast->getPreprocessor().getIdentifierTable();
  auto info = symtab.find(name);
  /* Return an empty DeclContextLookupResult if the identifier is not found */
  if (info == symtab.end())
    return DeclContextLookupResult();

  return Get_Decl_From_Symtab(ast, info->getValue());
}
