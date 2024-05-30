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
