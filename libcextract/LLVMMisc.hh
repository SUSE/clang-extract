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

#pragma once

#include <system_error>
#include <clang/Tooling/Tooling.h>
#include "clang/Analysis/CallGraph.h"
#include "clang/Sema/IdentifierResolver.h"
#include "clang/AST/DeclContextInternals.h"

#include "NonLLVMMisc.hh"

class SymbolNotFoundException : public std::exception {
  public:
  SymbolNotFoundException(std::string name)
    : Name(name)
  {}

  virtual const char *what() const noexcept
  {
    return Name.c_str();
  }

  private:
  std::string Name;
};

using namespace clang;

/** Check if Decl is a builtin.  */
bool Is_Builtin_Decl(const Decl *decl);

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
CallGraph *Build_CallGraph_From_AST(ASTUnit *ast);

/* Look into previous versions of `decl` for a declaration in the AST without
   a body.  */
FunctionDecl *Get_Bodyless_Decl(FunctionDecl *decl);
TagDecl      *Get_Bodyless_Decl(TagDecl *decl);
Decl         *Get_Bodyless_Decl(Decl *decl);

/* Look into previous versions of `decl` for a declaration in the AST without
   a body.  If not found, it returns itself (the version with body).  */
FunctionDecl *Get_Bodyless_Or_Itself(FunctionDecl *decl);
TagDecl      *Get_Bodyless_Or_Itself(TagDecl *decl);
Decl         *Get_Bodyless_Or_Itself(Decl *decl);

/** Get version of declarator with body.  */
DeclaratorDecl *Get_With_Body(DeclaratorDecl *decl);

/** Get version of declarator with body or itself.  */
DeclaratorDecl *Get_With_Body_Or_Itself(DeclaratorDecl *decl);

/* Get the TopLevel Decl that contains the location loc.  */
Decl *Get_Toplevel_Decl_At_Location(ASTUnit *ast, const SourceLocation &loc);

/* Get Toplevel decls with same beginloc.  */
ArrayRef<Decl *> Get_Toplev_Decls_With_Same_Beginloc(ASTUnit *ast, const SourceLocation &loc);

/** Build a clang-extract location comment.  */
std::string Build_CE_Location_Comment(SourceManager &sm, const SourceLocation &loc);

/** Get the begin location of the Decl before its comment if it have one.  */
SourceLocation Get_Begin_Loc_Of_Decl_Or_Comment(ASTContext &ctx, Decl *decl);

/** Get decl clang-extract location comment, or build one if it doesn't exist.  */
std::string Get_Or_Build_CE_Location_Comment(ASTContext &ctx, Decl *decl);

/** Check if Decl have a Location comment.  */
bool Have_Location_Comment(const SourceManager &sm, RawComment *comment);

/** Lookup in the symbol table for a declaration with given name passed by info.  */
DeclContextLookupResult Get_Decl_From_Symtab(ASTUnit *ast, const IdentifierInfo *info);

/** Lookup in the symbol table for a declaration with given name passed by name.  */
DeclContextLookupResult Get_Decl_From_Symtab(ASTUnit *ast, const StringRef &name);

/** Check if two Decls are equivalent.  */
bool Is_Decl_Equivalent_To(Decl *a, Decl *b);

bool Range_Fully_Contains_Range(ASTUnit *ast, const SourceRange &a,
                                const SourceRange &b);
