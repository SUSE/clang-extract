//===- Main.cpp - Clang-extract entry point                    --*- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Clang-extract entry point.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#include "ArgvParser.hh"
#include "Passes.hh"
#include "Error.hh"

#include <iostream>

using namespace llvm;
using namespace clang;

int main(int argc, char **argv)
{
  ArgvParser args(argc, argv);

  auto func_extract_names = args.Get_Functions_To_Extract();

  if (func_extract_names.size() == 0) {
    DiagsClass::Emit_Error("No function to extract.\n"
                           "pass -DCE_EXTRACT_FUNCTIONS=func<1>,...,func<n> to determine which functions to extract.");

    return 1;
  }

  return PassManager().Run_Passes(args);
}
