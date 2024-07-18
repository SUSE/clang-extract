//===- DependencyGraph.hh - Dependency Graph of Decls and Macros for extraction *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Compute a dependency graph of Decls and Macros necessary to extract a certain
/// Decl.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#pragma once

#include <clang/Frontend/ASTUnit.h>
#include <llvm/ADT/DenseMap.h>
#include <vector>
#include <stdio.h>

using namespace clang;

class DependencyGraph
{
  public:
  DependencyGraph(ASTUnit *ast)
    : AST(ast),
      DeclMap(10000)
  {
    Build_Dependency_Graph();
  }

  ~DependencyGraph(void);

  class DependencyEdge;

  class DependencyNode
  {
    public:
    DependencyNode(Decl *decl)
      : AsDecl(decl),
        Type(NodeType::DECL)
    {
    }

    DependencyNode(PreprocessedEntity *entity)
      : AsPrep(entity),
        Type(NodeType::PREPROCESSED_ENTITY)
    {
    }

    enum NodeType
    {
      INVALID,
      DECL,
      PREPROCESSED_ENTITY,
    };


    inline Decl *getAsDecl(void)
    {
      return (Type == NodeType::DECL) ? AsDecl : nullptr;
    }

    inline PreprocessedEntity *getAsPreprocessedEntity(void)
    {
      return (Type == NodeType::PREPROCESSED_ENTITY) ? AsPrep : nullptr;
    }

    inline std::vector<DependencyEdge *> &getBackwardEdges(void)
    {
      return BackwardEdges;
    }

    inline const std::vector<DependencyEdge *> &getBackwardEdges(void) const
    {
      return BackwardEdges;
    }

    inline std::vector<DependencyEdge *> &getForwardEdges(void)
    {
      return ForwardEdges;
    }

    inline const std::vector<DependencyEdge *> &getForwardEdges(void) const
    {
      return ForwardEdges;
    }

    DependencyEdge *getForwardEdgeAdjacentTo(DependencyNode *);
    DependencyEdge *getBackwardEdgeAdjacentTo(DependencyNode *);

    StringRef getName(void) const;

    void dumpSingleNode(FILE *f = stdout);

    friend class DependencyGraph;

    protected:
    union
    {
      Decl *AsDecl;
      PreprocessedEntity *AsPrep;
    };

    NodeType Type;

    std::vector<DependencyEdge *> BackwardEdges;
    std::vector<DependencyEdge *> ForwardEdges;
  };

  class DependencyEdge
  {
    public:
    inline DependencyEdge(DependencyNode *backward, DependencyNode *forward)
      : Backward(backward),
        Forward(forward)
    {
    }

    inline DependencyNode *getBackward(void)
    {
      return Backward;
    }

    inline DependencyNode *getForward(void)
    {
      return Forward;
    }

    inline const DependencyNode *getBackward(void) const
    {
      return Backward;
    }

    inline const DependencyNode *getForward(void) const
    {
      return Forward;
    }

    inline void setBackward(DependencyNode *node)
    {
      Backward = node;
    }

    inline void setForward(DependencyNode *node)
    {
      Forward = node;
    }

    friend class DependencyGraph;

    protected:
    DependencyNode *Backward;
    DependencyNode *Forward;
  };

  DependencyNode *getOrCreateDependencyNode(Decl *);

  DependencyNode *getDependencyNode(Decl *);

  DependencyEdge *createDependencyEdge(DependencyNode *back, DependencyNode *foward);

  void dumpGraphviz(FILE *f = stdout);

  private:
  void Build_Dependency_Graph(void);

  ASTUnit *AST;

  llvm::DenseMap<Decl *, DependencyNode *> DeclMap;
  std::vector<DependencyEdge *> EdgePool;
  std::vector<DependencyNode *> NodePool;
};

typedef DependencyGraph::DependencyNode DependencyNode;
typedef DependencyGraph::DependencyEdge DependencyEdge;
