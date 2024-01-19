//===- DscFileGenerator.hh - Generate the libpulp .dsc file -----*- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Generate the libulp .dsc file for a single userspace livepatch.
//
//===----------------------------------------------------------------------===//

/** DscFileGenerator: Generate libpulp .dsc file to be incorporated into livepatch
  *                   object.
  *
  * Libpulp expects a .dsc file to be provided into the livepatch container
  * (the .so file that is loaded with `ulp trigger`) into the `.ulp` section.
  * This section is merged with the .so file using `ulp packer`.
  *
  * Author: Giuliano Belinassi
  */

#pragma once

#include "SymbolExternalizer.hh"

#include <string>
#include <vector>

using namespace clang;


/** Class wrapping the analysis functions.  */
class DscFileGenerator
{
  public:
  DscFileGenerator(const std::string &output,
                   ASTUnit *ast,
                   const std::vector<std::string> &funcs_to_extract,
                   const std::vector<ExternalizerLogEntry> &log,
                   InlineAnalysis &ia);

  private:
  /** Actually generates the file.  */
  void Run_Analysis(void);

  /** Generate the .dsc header specifying the target ELF objects.  */
  void Target_ELF(void);

  /** Generate the list of global functions to be patched.  */
  void Global_Functions(void);

  /** Generate the list of local symbols to be patched.  */
  void Local_Symbols(void);


  /** Stores the error code resulting from trying to open the target file.  */
  std::error_code EC;

  /** Path to output. */
  std::string OutputPath;

  /** LLVM file stream object.  */
  llvm::raw_fd_ostream Out;

  /** The AST object.  */
  ASTUnit *AST;

  /** The list of functions that the user requested to be extracted.  */
  const std::vector<std::string> &FuncsToExtract;

  /** A log of changes done by the SymbolExternalizer.  */
  const std::vector<ExternalizerLogEntry> &ExternalizerLog;

  /** InlineAnalysis object.  */
  InlineAnalysis &IA;
};
