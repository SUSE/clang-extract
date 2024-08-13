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
#include <llvm/ADT/SmallVector.h>
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
    buildDependencyGraph();
  }

  ~DependencyGraph(void);

  class DependencyEdge;

  class DependencyNode
  {
    public:
    DependencyNode(Decl *decl)
      : Aux(nullptr),
        AsDecl(decl),
        Type(NodeType::DECL)
    {
    }

    DependencyNode(PreprocessedEntity *entity)
      : Aux(nullptr),
        AsPrep(entity),
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

    inline bool isDecl(void) const
    {
      return Type == NodeType::DECL;
    }

    inline bool isPreprocessedEntity(void) const
    {
      return Type == NodeType::PREPROCESSED_ENTITY;
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

    inline void mark(void *val = (void *)1)
    {
      Aux = val;
    }

    inline bool isMarked(void)
    {
      return Aux != nullptr;
    }

    inline void unmark(void)
    {
      Aux = nullptr;
    }

    DependencyEdge *getForwardEdgeAdjacentTo(DependencyNode *);
    DependencyEdge *getBackwardEdgeAdjacentTo(DependencyNode *);

    StringRef getName(void) const;

    void getDeclsDependingOnMe(llvm::SmallVector<Decl *> &vec);
    void getReachableBackwardEdges(llvm::SmallVector<DependencyEdge *> &vec);

    void dumpSingleNode(FILE *f = stdout) const;
    void dumpGraphviz(FILE *f = stdout, int depth = 0);

    friend class DependencyGraph;

    void *Aux;

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

  class DependencyLabel
  {
    public:

    enum DepType {
      NONE,
      DECL_REF_EXPR,
      TAG_TYPE,
      TYPEDEF_TYPE,
    };

    inline DependencyLabel(void)
      : Info(nullptr),
        Type(NONE)
    {
    }

    inline DependencyLabel(DeclRefExpr *expr)
      : Info(expr),
        Type(DECL_REF_EXPR)
    {
    }

    inline DependencyLabel(TagType *type)
      : Info(type),
        Type(TAG_TYPE)
    {
    }

    inline DependencyLabel(TypedefType *type)
      : Info(type),
        Type(TYPEDEF_TYPE)
    {
    }

    inline DepType getType(void)
    {
      return Type;
    }

    inline bool isDeclRefExpr(void)
    {
      return Type == DECL_REF_EXPR;
    }

    inline bool isTagType(void)
    {
      return Type == TAG_TYPE;
    }

    inline bool isTypedefType(void)
    {
      return Type == TYPEDEF_TYPE;
    }

    inline DeclRefExpr *getAsDeclRefExpr(void)
    {
      return isDeclRefExpr() ? static_cast<DeclRefExpr *>(Info) : nullptr;
    }

    inline TagType *getAsTagType(void)
    {
      return isTagType() ? static_cast<TagType *>(Info) : nullptr;
    }

    inline TypedefType *getAsTypedefDecl(void)
    {
      return isTypedefType() ? static_cast<TypedefType *>(Info) : nullptr;
    }

    protected:
    void *Info;
    DepType Type;
  };

  class DependencyEdge
  {
    public:

    inline DependencyEdge(DependencyNode *backward, DependencyNode *forward)
      : Backward(backward),
        Forward(forward)
    {
    }

    template <typename LABEL>
    inline DependencyEdge(DependencyNode *backward, DependencyNode *forward, LABEL label)
      : Backward(backward),
        Forward(forward),
        Label(label)
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

    inline DependencyNode *getBackward(void) const
    {
      return Backward;
    }

    inline DependencyNode *getForward(void) const
    {
      return Forward;
    }

    inline DependencyLabel &getLabel(void)
    {
      return Label;
    }

    inline const DependencyLabel &getLabel(void) const
    {
      return Label;
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
    DependencyLabel Label;
  };

  DependencyNode *getOrCreateDependencyNode(Decl *);

  DependencyNode *getDependencyNode(Decl *);

  template <typename LABEL>
  DependencyEdge *createDependencyEdge(DependencyNode *back,
                                       DependencyNode *foward,
                                       LABEL label);

  DependencyEdge *createDependencyEdge(DependencyNode *back,
                                       DependencyNode *foward);

  llvm::SmallVector<Decl *> getDeclsDependingOn(DependencyNode *node);

  void dumpGraphviz(FILE *f = stdout);
  void dumpGraphviz(const std::string &name, FILE *f = stdout);

  private:
  void buildDependencyGraph(void);

  void unmarkAllNodes(void);

  ASTUnit *AST;

  std::unordered_map<Decl *, DependencyNode *> DeclMap;
  std::vector<DependencyEdge *> EdgePool;
  std::vector<DependencyNode *> NodePool;
};

typedef DependencyGraph::DependencyNode DependencyNode;
typedef DependencyGraph::DependencyEdge DependencyEdge;
