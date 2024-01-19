/*
 *  clang-extract - Extract functions from projects and its dependencies using
 *                  libclang and LLVM infrastructure.
 *
 *  Copyright (C) 2024 SUSE Software Solutions GmbH
 *
 *  This file is part of clang-extract.
 *
 *  clang-extract is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  clang-extract is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with clang-extract.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Author: Giuliano Belinassi  */

#include "InlineAnalysis.hh"

#include "clang/Analysis/CallGraph.h"
#include "clang/Frontend/ASTUnit.h"
#include "InlineAnalysis.hh"

#include <vector>
#include <unordered_set>

using namespace clang;

class FunctionExternalizeFinder
{
  public:
  FunctionExternalizeFinder(ASTUnit *ast,
                            std::vector<std::string> &to_extract,
                            std::vector<std::string> &to_exernalize,
                            bool keep_includes,
                            InlineAnalysis &IA);

  bool Should_Externalize(CallGraphNode *node);
  bool Should_Externalize(const DeclaratorDecl *decl);

  void Run_Analysis(void);

  std::vector<std::string> Get_To_Externalize(void);

  protected:

  bool Analyze_Node(CallGraphNode *);
  bool Analyze_Function(FunctionDecl *);

  bool Externalize_DeclRefs(FunctionDecl *decl);
  bool Externalize_DeclRefs(Stmt *stmt);

  inline bool Must_Not_Externalize(const std::string &name)
  {
    return MustNotExternalize.find(name) != MustNotExternalize.end();
  }

  inline bool Must_Not_Externalize(const DeclaratorDecl *decl)
  {
    return Must_Not_Externalize(decl->getName().str());
  }

  inline bool Is_Marked_For_Externalization(const std::string &name)
  {
    return MustExternalize.find(name) != MustExternalize.end();
  }

  inline bool Mark_For_Externalization(const std::string &name)
  {
    bool externalized = false;

    if (!Is_Marked_For_Externalization(name)) {
      externalized = true;
      MustExternalize.insert(name);
    }

    return externalized;
  }

  std::unordered_set<std::string> MustExternalize;
  std::unordered_set<std::string> MustNotExternalize;
  std::unordered_set<std::string> ToExtract;

  inline bool Should_Extract(const std::string &name)
  {
    return ToExtract.find(name) != ToExtract.end();
  }

  inline bool Should_Extract(const FunctionDecl *func)
  {
    return Should_Extract(func->getName().str());
  }

  std::unordered_set<const CallGraphNode *> AnalyzedNodes;
  bool Is_Already_Analyzed(const CallGraphNode* node)
  {
    return AnalyzedNodes.find(node) != AnalyzedNodes.end();
  }

  bool KeepIncludes;
  ASTUnit *AST;
  InlineAnalysis &IA;
};
