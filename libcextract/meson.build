##===- meson.build - Rules for building libcextract *- meson ------------*-===##
##
## This project is licensed under the Apache License v2.0 with LLVM Exceptions.
## See https://llvm.org/LICENSE.txt for license information.
## SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
##
##===----------------------------------------------------------------------===##
##
### \file
### Rules and scripts for building libcextract.
##
##===----------------------------------------------------------------------===##

# Author: Marcos Paulo de Souza

libcextract_sources = [
  'ArgvParser.cpp',
  'DscFileGenerator.cpp',
  'ElfCXX.cpp',
  'Error.cpp',
  'FunctionDepsFinder.cpp',
  'FunctionExternalizeFinder.cpp',
  'IncludeTree.cpp',
  'InlineAnalysis.cpp',
  'IpaClonesParser.cpp',
  'LLVMMisc.cpp',
  'MacroWalker.cpp',
  'NonLLVMMisc.cpp',
  'Passes.cpp',
  'PrettyPrint.cpp',
  'SymbolExternalizer.cpp',
  'SymversParser.cpp',
  'TopLevelASTIterator.cpp',
  'ExpansionPolicy.cpp',
  'HeaderGenerate.cpp',
  'Closure.cpp',
  'ASTUnitHack.cpp',
  'DependencyGraph.cpp'
]

libcextract_static = static_library('cextract', libcextract_sources)
