//===- ArgvParser.hh - Parse the command-line options -----------*- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Parse the command-line options given to clang-extract.
//
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include <vector>

/** Class encapsulating the Argv command line that were provided to clang-extract
 *
 * clang-extract must accept the original software command line call with some
 * extra arguments to specify the special options for clang-extract.  Those
 * options should be also acceptable by gcc and clang compilers, so the user
 * can simply swap the compiler to clang-extract instead of gcc in the
 * makefiles by overwriting the CC variable.
 *
 * Author: Giuliano Belinassi
 */
class ArgvParser
{
  public:

  /** Build from argc and argv passed to clang-extract.  */
  ArgvParser(int argc, char **argv);


  inline std::vector<const char *>& Get_Args_To_Clang(void)
  {
    return ArgsToClang;
  }

  inline const std::vector<const char *>& Get_Args_To_Clang(void) const
  {
    return ArgsToClang;
  }

  inline std::vector<std::string>& Get_Functions_To_Extract(void)
  {
    return FunctionsToExtract;
  }

  inline std::vector<std::string>& Get_Symbols_To_Externalize(void)
  {
    return SymbolsToExternalize;
  }

  inline std::vector<std::string>& Get_Headers_To_Expand(void)
  {
    return HeadersToExpand;
  }

  inline std::string &Get_Output_File(void)
  {
    return OutputFile;
  }

  inline bool Is_Externalization_Disabled(void)
  {
    return DisableExternalization;
  }

  inline bool Should_Keep_Includes(void)
  {
    return WithIncludes;
  }

  inline bool Should_Dump_Passes(void)
  {
    return DumpPasses;
  }

  inline bool Is_Kernel(void)
  {
    return Kernel;
  }

  inline bool Has_Ibt(void)
  {
    return Ibt;
  }

  inline std::string Get_PatchObject(void)
  {
      return PatchObject;
  }

  inline const char *Get_Debuginfo_Path(void)
  {
    return DebuginfoPath;
  }

  inline const char *Get_Ipaclones_Path(void)
  {
    return IpaclonesPath;
  }

  inline const char *Get_Symvers_Path(void)
  {
    return SymversPath;
  }

  inline const char *Get_Dsc_Output_Path(void)
  {
    return DescOutputPath;
  }

  inline bool Should_Rename_Symbols(void)
  {
    return RenameSymbols;
  }

  inline const char *Get_Include_Expansion_Policy(void)
  {
    return IncExpansionPolicy;
  }

  inline const char *Get_Output_Path_To_Prototype_Header(void)
  {
    return OutputFunctionPrototypeHeader;
  }

  const char *Get_Input_File(void);

  /** Print help usage message.  */
  void Print_Usage_Message(void);

  private:
  bool Handle_Clang_Extract_Arg(const char *str);
  void Insert_Required_Parameters(void);

  std::vector<const char *> ArgsToClang;

  std::vector<std::string> FunctionsToExtract;
  std::vector<std::string> SymbolsToExternalize;
  std::vector<std::string> HeadersToExpand;
  std::string OutputFile;

  bool DisableExternalization;
  bool WithIncludes;
  bool DumpPasses;
  bool RenameSymbols;
  bool Kernel;
  /* If the file was compiled with IBT support */
  bool Ibt;
  std::string PatchObject;

  const char *DebuginfoPath;
  const char *IpaclonesPath;
  const char *SymversPath;

  const char *DescOutputPath;

  const char *IncExpansionPolicy;

  const char *OutputFunctionPrototypeHeader;
};
