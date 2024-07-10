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

/* Get the TopLevel Decl that contains the location loc.  */
Decl *Get_Toplevel_Decl_At_Location(ASTUnit *ast, const SourceLocation &loc);
