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

#include "EnumConstTbl.hh"
#include "MacroWalker.hh"
#include "IncludeTree.hh"
#include "PrettyPrint.hh"
#include "Passes.hh"

#include <clang/Tooling/Tooling.h>
#include <clang/Analysis/CallGraph.h>
#include <unordered_set>

using namespace clang;

class ClosureSet
{
  public:
  /** Check if a given declaration was already marked as dependency.  */
  inline bool Is_Decl_Marked(Decl *decl)
  { return Dependencies.find(decl) != Dependencies.end(); }

  /** Mark decl as dependencies and all its previous decls versions.  */
  bool Add_Decl_And_Prevs(Decl *decl);

  /** Add a single decl to the set.  */
  void Add_Single_Decl(Decl *decl)
  {
    Dependencies.insert(decl);
  }

  /*
   * Check if the same Decl was already added into the closure but for a
   * different typename.
   */
  Decl *insideRangeOfDecl(Decl *decl)
  {
    SourceRange range = decl->getSourceRange();

    /* FIXME: optimize this loop */
    for (auto it = Dependencies.begin(); it != Dependencies.end(); ++it) {
      if (TypedefDecl *cur_decl = dyn_cast<TypedefDecl>(*it)) {
        SourceRange cur_range = cur_decl->getSourceRange();

        // Avoid comparing to itself
        if (cur_range == range)
          continue;

        if (PrettyPrint::Contains_From_LineCol(range, cur_range))
          return cur_decl;
      }
    }

    return nullptr;
  }

  inline std::unordered_set<Decl *> &Get_Set(void)
  {
    return Dependencies;
  }

  inline void Remove_Decl(Decl *decl)
  {
    Dependencies.erase(decl);
  }

  private:
  /** Datastructure holding all Decls required for the functions. This is
      then used to mark which Decls we need to output.

      We use a unordered_set because we need quick lookup, so a hash table
      may be (?) the ideal datastructure for this.  */
  std::unordered_set<Decl*> Dependencies;
};

class DeclClosureVisitor;

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
    ~FunctionDependencyFinder(void);

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

    /** Datastructure holding all Decls required for the functions. This is
        then used to mark which Decls we need to output.

        We use a unordered_set because we need quick lookup, so a hash table
        may be (?) the ideal datastructure for this.  */
    ClosureSet Closure;

    /** The AST that are being used in our analysis.  */
    ASTUnit* AST;

    /** Object holding information about constant enum symbols and a mapping to
        they original enum object.  */
    EnumConstantTable EnumTable;

    /* Tree of #includes.  */
    IncludeTree IT;

    /* Should we keep the includes when printing?  */
    bool KeepIncludes;

    /* Visitor that sweeps through the AST.  Kept as pointer to avoid declaring
       the DeclClosureVisitor class into this .h to speedup build time.  */
    DeclClosureVisitor *Visitor;
};
