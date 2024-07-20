//===- FunctionExternalizeFinder.hh - Find symbols to be externalized *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Finds which symbols must be externalized when extracting functions.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#pragma once

#include "MacroWalker.hh"
#include "IncludeTree.hh"
#include "PrettyPrint.hh"
#include "Closure.hh"
#include "Passes.hh"

#include <clang/Tooling/Tooling.h>
#include <clang/Analysis/CallGraph.h>
#include <unordered_set>

using namespace clang;

/** Function Dependency Finder.
 *
 * This class wraps all code necessary to find all Decl in the
 * Abstract Syntax Tree that is necessary for a given function to
 * compile.
 *
 * As example, assume we have the following source code:
 *
 * int f();
 * int g()
 * {
 *   f();
 * }
 *
 * int h()
 * {
 * }
 *
 *
 * If we want to extract function `g`, we have to extract `f` as well, once it
 * is called by `g`. OTOH, if we want to only extract `h`, we don't need to do
 * anything else since this function doesn't call anything and can be compiled
 * by itself.
 *
 */
class FunctionDependencyFinder
{
  public:
    FunctionDependencyFinder(PassManager::Context *);

    /** Print the marked nodes as they appear in the AST.  */
    void Print(void);

    /** Run the analysis on function `function`*/
    bool Run_Analysis(std::vector<std::string> const &function);

  protected:

    /** Given a list of functions in `funcnames`, compute the closure of those
        functions.  That means find all Types, functions, and global variables
        that are reachable from those functions.  */
    bool Find_Functions_Required(std::vector<std::string> const &funcnames);

    /** Insert Decls that comes from #include's which are not expanded.  This
        is necessary to make sure we don't remove decls that are used in the
        header program.  */
    void Insert_Decls_From_Non_Expanded_Includes(void);

    /* Remove redundant decls, whose source location is inside another decl.  */
    void Remove_Redundant_Decls();

    /** The AST that are being used in our analysis.  */
    ASTUnit* AST;

    /* Tree of #includes.  */
    IncludeTree IT;

    /* Should we keep the includes when printing?  */
    bool KeepIncludes;

    /* Visitor that sweeps through the AST.  Kept as pointer to avoid declaring
       the DeclClosureVisitor class into this .h to speedup build time.  */
    DeclClosureVisitor Visitor;
};
