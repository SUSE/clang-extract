//===- TreeDump.cpp - Dump IncludeTree to stdout               --*- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Entrypoint of the IncludeTree dump tool.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#include "ArgvParser.hh"
#include "Passes.hh"
#include "Error.hh"

#include <iostream>
#include <stdio.h>

using namespace llvm;
using namespace clang;

void print_usage_message(void)
{
  llvm::outs() <<
"OVERVIEW: Tool to show the tree of includes of a C/C++ file.\n"
"          It should be invoked as a C/C++ compiler.\n"
"\n"
"USAGE: ce-includetree [options] file...\n"
"\n"
"CLANG-EXTRACT OPTIONS:\n"
"   <clang-switch>          A clang switch, as specified by calling clang --help.\n"
"  -D__KERNEL__             Indicate that we are processing a Linux sourcefile.\n"
"  -DCE_KEEP_INCLUDES       Keep all possible #include<file> directives.\n"
"  -DCE_KEEP_INCLUDES=<policy>\n"
"                           Keep all possible #include<file> directives, but using the\n"
"                           specified include expansion <policy>.  Valid values are\n"
"                           nothing, everything, kernel, system and compiler.\n"
"  -DCE_EXPAND_INCLUDES=<args>\n"
"                           Force expansion of the headers provided in <args>.\n"
"  -DCE_NOT_EXPAND_INCLUDES=<args>\n"
"                           Force the following headers to NOT be expanded.\n"
"\n";
}

int main(int argc, char **argv)
{
  ArgvParser args(argc, argv);
  PassManager::Context ctx(args);

  if (argc <= 1) {
    print_usage_message();
    return 0;
  }

  /* Cleverly silence any error from clang.  */
  fclose(stderr);
  if (Build_ASTUnit(&ctx) == false){
    llvm::outs() << "Unable to create ASTUnit of " << ctx.InputPath << '\n';
    return 1;
  }

  const DiagnosticsEngine &de = ctx.AST->getDiagnostics();
  if (de.hasErrorOccurred()) {
    llvm::outs() << "ASTUnit of " << ctx.InputPath << " contain errors. Aborting.\n";
    return 1;
  }

  IncludeTree IT(ctx.AST.get(), ctx.IncExpansionPolicy, ctx.HeadersToExpand,
                 ctx.HeadersToNotExpand);
  IT.Dump(llvm::outs());

  return 0;
}
