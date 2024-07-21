//===- FunctionExternalizeFinder.cpp - Find symbols to be externalized *- C++ -*-===//
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

#include "FunctionExternalizeFinder.hh"
#include "FunctionDepsFinder.hh"
#include "LLVMMisc.hh"

FunctionExternalizeFinder::FunctionExternalizeFinder(ASTUnit *ast,
                                                std::vector<std::string> &to_extract,
                                                std::vector<std::string> &to_externalize,
                                                bool keep_includes,
                                                InlineAnalysis &ia)
  : MustExternalize(to_externalize.begin(), to_externalize.end()),
    MustNotExternalize(to_extract.begin(), to_extract.end()),
    ToExtract(to_extract.begin(), to_extract.end()),
    KeepIncludes(keep_includes),
    AST(ast),
    IA(ia)
{
  Run_Analysis();

  /* Compute intersection.  They must be empty.  */
  std::unordered_set<std::string> c;
  for (const std::string &e : MustNotExternalize) {
    if (MustExternalize.count(e) > 0) {
      c.insert(e);
    }
  }

  assert(c.size() == 0 && "Exernalize INTERSECTION Export is not empty.");
}

bool FunctionExternalizeFinder::Should_Externalize(CallGraphNode *node)
{
  return Should_Externalize(dyn_cast<FunctionDecl *>(node->getDecl()));
}

bool FunctionExternalizeFinder::Should_Externalize(const DeclaratorDecl *decl)
{
  if (!decl)
    return false;

  if (Must_Not_Externalize(decl)) {
    return false;
  }

  /* Builtin stuff inserted by the compiled should never be externalized.  */
  if (Is_Builtin_Decl(decl)) {
    return false;
  }

  /* In case the declarator is a function.  */
  if (const FunctionDecl *func = dyn_cast<const FunctionDecl>(decl)) {

    /* If the function is marked for extraction then we don't externalize it.  */
    if (Should_Extract(func)) {
      return false;
    }

    /* If the function is declared inline then do not externalize it.  */
    if (func->isInlined()) {
      return false;
    }

    /* TODO: Get mangled name.  */
    const std::string &func_name = decl->getNameAsString();
    if (IA.Needs_Externalization(func_name) == ExternalizationType::NONE) {
      /* No need for externalization.  */
      return false;
    } else {
      /* Externalization is necessary.  */
      return true;
    }
  }

  /* In case the declarator is a variable.  */
  if (const VarDecl *var = dyn_cast<const VarDecl>(decl)) {

    /* If the variable is not global there is no point in externalizing it.  */
    if (!var->hasGlobalStorage()) {
      return false;
    }

    /* If the symbol needs to be externalized, then do it.  */
    const std::string &var_name = decl->getNameAsString();
    if (IA.Needs_Externalization(var_name) == ExternalizationType::NONE) {
      /* No need for externalization.  */
      return false;
    } else {
      /* Externalization is necessary.  */
      return true;
    }
  }

  /* Not a function or variable?  */
  return false;
}

bool FunctionExternalizeFinder::Analyze_Function(FunctionDecl *decl)
{
  if (!decl)
    return false;

  if (Should_Externalize(decl)) {
    Mark_For_Externalization(decl->getName().str());
    return true;
  } else {
    /* Else check if we need to externalize any of its variables.  */
    Externalize_DeclRefs(decl);
    return false;
  }
}

bool FunctionExternalizeFinder::Analyze_Node(CallGraphNode *node)
{
  FunctionDecl *decl = dyn_cast<FunctionDecl *>(node->getDecl());

  if (Is_Already_Analyzed(node)) {
    return false;
  }

  AnalyzedNodes.insert(node);

  if (Analyze_Function(decl)) {
    return true;
  } else {
    /* Continue looking.  */
    CallGraphNode::const_iterator child_it;
    for (child_it = node->begin(); child_it != node->end(); ++child_it) {
      CallGraphNode *child = child_it->Callee;
      Analyze_Node(child);
    }
    return false;
  }
}

bool FunctionExternalizeFinder::Externalize_DeclRefs(FunctionDecl *decl)
{
  if (!decl)
    return false;

  bool externalized = false;

  Stmt *body = decl->getBody();
  if (body) {
    externalized = Externalize_DeclRefs(body);
  }

  return externalized;
}

bool FunctionExternalizeFinder::Externalize_DeclRefs(Stmt *stmt)
{
  if (!stmt)
    return false;

  bool externalized = false;

  if (DeclRefExpr::classof(stmt)) {
    DeclRefExpr *expr = (DeclRefExpr *) stmt;
    DeclaratorDecl *decl = dyn_cast<DeclaratorDecl *>(expr->getDecl());

    if (Should_Externalize(decl)) {
      externalized = Mark_For_Externalization(decl->getNameAsString());
    }
  }

  /* Iterate through the list of all children Statements and repeat the
     process.  */
  clang::Stmt::child_iterator it, it_end;
  for (it = stmt->child_begin(), it_end = stmt->child_end();
      it != it_end; ++it) {

    Stmt *child = *it;
    externalized |= Externalize_DeclRefs(child);
  }

  return externalized;
}

void FunctionExternalizeFinder::Run_Analysis(void)
{
  //CallGraphWithBodylessFunctions *cg = Build_CallGraph_From_AST(AST);
  CallGraph *cg = Build_CallGraph_From_AST(AST);

  for (auto it = cg->begin(); it != cg->end(); ++it) {
    CallGraphNode *node = (*it).getSecond().get();
    if (node->getDecl()) {
      if (FunctionDecl *func = node->getDefinition()) {
        if (Should_Extract(func)) {
          Analyze_Node(node);
        }
      }
    }
  }

  delete cg;
}

std::vector<std::string> FunctionExternalizeFinder::Get_To_Externalize(void)
{
  std::vector<std::string> externalize;

  for (const std::string &name : MustExternalize) {
    externalize.push_back(name);
  }

  return externalize;
}
