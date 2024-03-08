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

#pragma once

#include "Passes.hh"
#include "FunctionDepsFinder.hh"
#include "SymbolExternalizer.hh"


using namespace clang;

/** Outputs a header file with a foward declarations of all functions in the current
 *  AST.
 *
 *  WARNING:  This class modifies the AST.
 */
class HeaderGeneration
{
  public:
  HeaderGeneration(PassManager::Context *);

  bool Run_Analysis(const std::vector<ExternalizerLogEntry> &set);

  void Print(void);

  protected:
  ASTUnit *AST;
  ClosureSet Closure;
};
