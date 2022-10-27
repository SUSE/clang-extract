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

  inline std::string &Get_Output_File(void)
  {
    return OutputFile;
  }

  inline bool Is_Externalization_Disabled(void)
  {
    return DisableExternalization;
  }

  inline bool Should_Dump_Passes(void)
  {
    return DumpPasses;
  }

  const char *Get_Input_File(void);

  private:
  bool Handle_Clang_Extract_Arg(const char *str);
  void Insert_Required_Parameters(void);

  std::vector<const char *> ArgsToClang;

  std::vector<std::string> FunctionsToExtract;
  std::vector<std::string> SymbolsToExternalize;
  std::string OutputFile;

  bool DisableExternalization;
  bool DumpPasses;
};
