//===- HeaderGenerate.hh - Output a header file for generated output. *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Remove the body of all functions and only outputs the foward declaration.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#include "HeaderGenerate.hh"
#include "FunctionDepsFinder.hh"
#include "IncludeTree.hh"
#include "PrettyPrint.hh"

HeaderGeneration::HeaderGeneration(PassManager::Context *ctx)
  : AST(ctx->AST.get())
{
  Run_Analysis(ctx->NamesLog);
}

void HeaderGeneration::Print(void)
{
  IncludeTree IT(AST); // Not necessary, just to use the RecursivePrint class.
  RecursivePrint(AST, Closure.Get_Set(), IT, false).Print();
}

static bool Contains(const std::vector<ExternalizerLogEntry> &v, const std::string &name)
{
  for (const ExternalizerLogEntry &x : v) {
    if ((x.Type == ExternalizationType::RENAME || x.Type == ExternalizationType::WEAK)
        && x.NewName == name) {
      return true;
    }
  }

  return false;
}

bool HeaderGeneration::Run_Analysis(const std::vector<ExternalizerLogEntry> &set)
{
  ASTUnit::top_level_iterator it;
  std::unordered_set<std::string> nameset;

  for (it = AST->top_level_begin(); it != AST->top_level_end(); ++it) {
    Decl *decl = *it;

    if (FunctionDecl *fdecl = dyn_cast<FunctionDecl>(decl)) {
      const std::string &fname = fdecl->getNameAsString();
      /* If the function was already issued, then do not issue it again.  */
      if (Contains(set, fname) && nameset.find(fname) == nameset.end()) {
        if (fdecl->doesThisDeclarationHaveABody() && fdecl->hasBody()) {
          Stmt *body = fdecl->getBody();
          fdecl->setRangeEnd(body->getBeginLoc().getLocWithOffset(-1));
          fdecl->setBody(nullptr);
        }
        Closure.Add_Single_Decl(fdecl);
        nameset.insert(fname);
      }
    }
  }

  /* Do not output any macros.  */
  MacroWalker mw(AST->getPreprocessor());
  PreprocessingRecord *rec = AST->getPreprocessor().getPreprocessingRecord();
  for (PreprocessedEntity *entity : *rec) {
    if (MacroDefinitionRecord *def = dyn_cast<MacroDefinitionRecord>(entity)) {
      if (MacroInfo *info = mw.Get_Macro_Info(def)) {
        info->setIsUsed(false);
      }
    }
  }

  return true;
}
