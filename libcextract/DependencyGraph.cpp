//===- DependencyGraph.cpp - Dependency Graph of Decls and Macros for extraction *- C++ -*-===//
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

#include "DependencyGraph.hh"
#include "PrettyPrint.hh"

#include <clang/AST/RecursiveASTVisitor.h>
#include <stack>

DependencyEdge *DependencyNode::getForwardEdgeAdjacentTo(DependencyNode *forward)
{
  for (DependencyEdge *edge : ForwardEdges) {
    if (edge->getForward() == forward) {
      return edge;
    }
  }

  return nullptr;
}

DependencyEdge *DependencyNode::getBackwardEdgeAdjacentTo(DependencyNode *backward)
{
  for (DependencyEdge *edge : BackwardEdges) {
    if (edge->getBackward() == backward) {
      return edge;
    }
  }

  return nullptr;
}

void DependencyNode::dumpSingleNode(FILE *file)
{
  for (const DependencyEdge *edge : BackwardEdges) {
    std::string back = edge->getBackward()->getName().str();
    std::string front = edge->getForward()->getName().str();

    fprintf(file, "\n\"%s (%lx)\" -> \"%s (%lx)\"\n", back.c_str(), (unsigned long)
                                                                    edge->getBackward(),
                                                     front.c_str(), (unsigned long)
                                                                    edge->getForward());
  }

  for (const DependencyEdge *edge : ForwardEdges) {
    std::string back = edge->getBackward()->getName().str();
    std::string front = edge->getForward()->getName().str();

    fprintf(file, "\n\"%s\" -> \"%s\"\n", back.c_str(), front.c_str());
  }
}

class DependencyGraphVisitor : public RecursiveASTVisitor<DependencyGraphVisitor>
{
  public:

  enum {
    VISITOR_CONTINUE = true,   // Return this for the AST transversal to continue;
    VISITOR_STOP     = false,  // Return this for the AST tranversal to stop completely;
  };

#define TRY_TO(CALL_EXPR)                    \
  if ((CALL_EXPR) == VISITOR_STOP)           \
    return VISITOR_STOP

  DependencyGraphVisitor(DependencyGraph &dg, ASTUnit *ast)
    : AST(ast),
      DG(dg),
      DependencyStack(std::stack<DependencyNode *>())
  {}

  bool TraverseDecl(Decl *decl)
  {
    bool ret;

    DependencyNode *curr = DG.getOrCreateDependencyNode(decl);
    DependencyNode *back = nullptr;
    if (!DependencyStack.empty()) {
      back = DependencyStack.top();
      DG.createDependencyEdge(back, curr);
    }

    DependencyStack.push(curr);
    ret = RecursiveASTVisitor::TraverseDecl(decl);
    DependencyStack.pop();

    return ret;
  }

  bool VisitDeclRefExpr(DeclRefExpr *expr)
  {
    DependencyNode *back = DependencyStack.top();
    DependencyNode *front = DG.getOrCreateDependencyNode(expr->getDecl());

    DG.createDependencyEdge(back, front);

    return VISITOR_CONTINUE;
  }

  bool VisitTagType(const TagType *type)
  {
    DependencyNode *back = DependencyStack.top();
    DependencyNode *front = DG.getOrCreateDependencyNode(type->getDecl());

    DG.createDependencyEdge(back, front);

    return VISITOR_CONTINUE;
  }

  bool VisitTypedefType(const TypedefType *type)
  {
    DependencyNode *back = DependencyStack.top();
    DependencyNode *front = DG.getOrCreateDependencyNode(type->getDecl());

    DG.createDependencyEdge(back, front);

    return VISITOR_CONTINUE;
  }

  private:
  ASTUnit *AST;
  DependencyGraph &DG;
  std::stack<DependencyNode *> DependencyStack;
};

void DependencyGraph::Build_Dependency_Graph(void)
{
  DependencyGraphVisitor visitor(*this, AST);
  visitor.TraverseDecl(AST->getASTContext().getTranslationUnitDecl());

  dumpGraphviz();

  exit(0);
}

DependencyGraph::~DependencyGraph(void)
{
  for (DependencyEdge *edge : EdgePool) {
    delete edge;
  }

  for (DependencyNode *node : NodePool) {
    delete node;
  }
}

DependencyNode *DependencyGraph::getOrCreateDependencyNode(Decl *decl)
{
  auto it = DeclMap.find(decl);

  if (it == DeclMap.end()) {
    DependencyNode *node = new DependencyNode(decl);
    NodePool.push_back(node);

    auto inserted = DeclMap.try_emplace(decl, node);
    assert(inserted.second && "Failed to insert decl into map");
    return node;
  }

  return it->second;
}

DependencyNode *DependencyGraph::getDependencyNode(Decl *decl)
{
  auto it = DeclMap.find(decl);

  if (it == DeclMap.end()) {
    return nullptr;
  }

  return it->second;
}

DependencyEdge *DependencyGraph::createDependencyEdge(DependencyNode *backward, DependencyNode *forward)
{
  DependencyEdge *b = backward->getForwardEdgeAdjacentTo(forward);
  DependencyEdge *f = forward->getBackwardEdgeAdjacentTo(backward);
  if (f && b && f == b) {
    /* Keep the graph simple.  */
    return f;
  }

  /* Allocate the edge object on our pool.  */
  DependencyEdge *edge = new DependencyEdge(backward, forward);
  EdgePool.push_back(edge);

  /* Link nodes.  */
  backward->ForwardEdges.push_back(edge);
  forward->BackwardEdges.push_back(edge);

  return edge;
}

StringRef DependencyNode::getName(void) const
{
  if (Type == NodeType::DECL) {
    if (NamedDecl *ndecl = dyn_cast<NamedDecl>(AsDecl)) {
      return ndecl->getName();
    } else if (TranslationUnitDecl *tdecl = dyn_cast<TranslationUnitDecl>(AsDecl)) {
      return StringRef("<TranslationUnit>");
    } else {
      return StringRef("<unnamed>");
    }
  } else {
    return StringRef("<not implemented>");
  }
}

void DependencyGraph::dumpGraphviz(FILE *file)
{
  fprintf(file, "strict digraph {");
  for (const DependencyEdge *edge : EdgePool) {
    std::string back = edge->getBackward()->getName().str();
    std::string front = edge->getForward()->getName().str();

    fprintf(file, "\n\"%s (%lx)\" -> \"%s (%lx)\"", back.c_str(), (unsigned long)
                                                                  edge->getBackward(),
                                                    front.c_str(), (unsigned long)
                                                                  edge->getForward());
  }
  fprintf(file, "\n}");
}
