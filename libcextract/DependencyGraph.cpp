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

void DependencyNode::dumpSingleNode(FILE *file) const
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

void DependencyNode::dumpGraphviz(FILE *file, int depth)
{
  if (Aux != nullptr) {
    return;
  }

  Aux = (void *) 1;

  if (depth == 0) {
    fprintf(file, "strict digraph {");
  }

  for (const DependencyEdge *edge : ForwardEdges) {
    DependencyNode *backward = edge->getBackward();
    DependencyNode *forward = edge->getForward();

    const std::string &back = backward->getName().str();
    const std::string &front = forward->getName().str();

    fprintf(file, "\n\"%s (%lx)\" -> \"%s (%lx)\"", back.c_str(), (unsigned long)
                                                                  edge->getBackward(),
                                                    front.c_str(), (unsigned long)
                                                                  edge->getForward());

    forward->dumpGraphviz(file, depth + 1);
  }

  if (depth == 0) {
    fprintf(file, "\n}");
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

    DG.createDependencyEdge(back, front, expr);

    return VISITOR_CONTINUE;
  }

  bool VisitTagType(TagType *type)
  {
    DependencyNode *back = DependencyStack.top();
    DependencyNode *front = DG.getOrCreateDependencyNode(type->getDecl());

    DG.createDependencyEdge(back, front, type);

    return VISITOR_CONTINUE;
  }

  bool VisitTypedefType(TypedefType *type)
  {
    DependencyNode *back = DependencyStack.top();
    DependencyNode *front = DG.getOrCreateDependencyNode(type->getDecl());

    DG.createDependencyEdge(back, front, type);

    return VISITOR_CONTINUE;
  }

  private:
  ASTUnit *AST;
  DependencyGraph &DG;
  std::stack<DependencyNode *> DependencyStack;
};

void DependencyGraph::buildDependencyGraph(void)
{
  DependencyGraphVisitor visitor(*this, AST);
  visitor.TraverseDecl(AST->getASTContext().getTranslationUnitDecl());
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

template <typename LABEL>
DependencyEdge *DependencyGraph::createDependencyEdge(DependencyNode *backward, DependencyNode *forward, LABEL label)
{
  /* Allocate the edge object on our pool.  */
  DependencyEdge *edge = new DependencyEdge(backward, forward, label);
  EdgePool.push_back(edge);

  /* Link nodes.  */
  backward->ForwardEdges.push_back(edge);
  forward->BackwardEdges.push_back(edge);

  return edge;
}

DependencyEdge *DependencyGraph::createDependencyEdge(DependencyNode *backward,
                                                      DependencyNode *forward)
{
  DependencyEdge *b = backward->getForwardEdgeAdjacentTo(forward);
  DependencyEdge *f = forward->getBackwardEdgeAdjacentTo(backward);
  if (f && b && f == b) {
    /* If we don't want to add a label, then there is no point in adding
       multiple edges to the same label.  */
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

llvm::SmallVector<Decl *> DependencyNode::getDeclsDependingOnMe(void)
{
  llvm::SmallVector<Decl *> vec;
  NodeActionFunction node_action = [&vec](DependencyNode *node) {
    if (Decl *decl = node->getAsDecl()) {
      vec.push_back(decl);
    }
  };

  EdgeActionFunction edge_action = [](DependencyEdge *edge) {
  };

  iterateBackwardDFS(node_action, edge_action);
  return vec;
}

llvm::SmallVector<DependencyEdge *> DependencyNode::getReachableBackwardEdges(void)
{
  llvm::SmallVector<DependencyEdge *> vec;
  NodeActionFunction node_action = [](DependencyNode *node) {
  };

  EdgeActionFunction edge_action = [&vec](DependencyEdge *edge) {
    vec.push_back(edge);
  };

  iterateForwardDFS(node_action, edge_action);
  return vec;
}

void DependencyNode::iterateForwardDFS(NodeActionFunction &node_action,
                                       EdgeActionFunction &edge_action)
{
  if (isMarked()) {
    return;
  }

  mark();
  node_action(this);

  for (DependencyEdge *edge : ForwardEdges) {
    edge_action(edge);
    DependencyNode *node = edge->getForward();
    if (node->isDecl()) {
      node->iterateForwardDFS(node_action, edge_action);
    }
  }
}

void DependencyNode::iterateBackwardDFS(NodeActionFunction &node_action,
                                        EdgeActionFunction &edge_action)
{
  if (isMarked()) {
    return;
  }

  mark();
  node_action(this);

  for (DependencyEdge *edge : ForwardEdges) {
    edge_action(edge);
    DependencyNode *node = edge->getForward();
    if (node->isDecl()) {
      node->iterateForwardDFS(node_action, edge_action);
    }
  }
}

void DependencyGraph::expandMarkedNodesUpwardsCtx(void)
{
  for (DependencyNode *node : NodePool) {
    if (node->Aux) {
      if (Decl *decl = node->getAsDecl()) {
        while (decl) {
          DependencyNode *n = getDependencyNode(decl);
          if (NamedDecl *ndecl = dyn_cast<NamedDecl>(decl))
            printf("Marking %s\n", ndecl->getName().str().c_str());
          n->mark();
          DeclContext *ctx = decl->getDeclContext();
          decl = ctx ? cast<Decl>(ctx) : nullptr;
        }
      }
    }
  }
}

std::unordered_set<Decl *> DependencyGraph::getMarkedNodesAsSet(void)
{
  std::unordered_set<Decl *> ret;
  for (DependencyNode *node : NodePool) {
    if (Decl *decl = node->getAsDecl(); node->Aux) {
      ret.insert(decl);
    }
  }

  return ret;
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

void DependencyGraph::dumpGraphviz(const std::string &name, FILE *file)
{
  for (DependencyNode *node : NodePool) {
    if (node->getName() == name) {
      node->dumpGraphviz(file);
      break;
    }
  }
  unmarkAllNodes();
}

void DependencyGraph::dumpMarkedNodes(FILE *f)
{
  for (DependencyNode *node : NodePool) {
    if (node->Aux) {
      if (NamedDecl *decl = dyn_cast<NamedDecl>(node->getAsDecl())) {
        printf("Name: %s\n", decl->getName().str().c_str());
      }
    }
  }
}

void DependencyGraph::unmarkAllNodes(void)
{
  for (DependencyNode *node : NodePool) {
    node->Aux = nullptr;
  }
}

llvm::SmallVector<Decl *> DependencyGraph::getDeclsDependingOn(DependencyNode *node)
{
  llvm::SmallVector<Decl *> ret = node->getDeclsDependingOnMe();
  unmarkAllNodes();

  return ret;
}

Decl *DependencyGraph::getDeclContextDominating(Decl *a, Decl *b)
{
  assert(a != b && "Called with a = b. Dominance is useless");

  DeclContext *ctx_a = dyn_cast<DeclContext>(a);
  DeclContext *ctx_b = dyn_cast<DeclContext>(b);
  bool a_doms = false, b_doms = false;

  // Check if a dominates b.
  if (ctx_a != nullptr) {
    while (b != nullptr) {
      if (ctx_a->containsDecl(b)) {
        a_doms = true;
        break;
      }

      b = cast<Decl>(b->getDeclContext());
    }
  }

  // Checks if b dominates a.
  if (ctx_b != nullptr) {
    while (a != nullptr) {
      if (ctx_b->containsDecl(a)) {
        b_doms = true;
        break;
      }

      a = cast<Decl>(a->getDeclContext());
    }
  }

  assert(!(a_doms && b_doms) && "Does this even makes sense?");
  if (a_doms) {
    return a;
  }

  if (b_doms) {
    return b;
  }

  return nullptr;
}
