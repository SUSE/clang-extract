#include "ArgvParser.hh"
#include "Passes.hh"

#include <iostream>

using namespace llvm;
using namespace clang;

int main(int argc, char **argv)
{
  ArgvParser args(argc, argv);

  auto func_extract_names = args.Get_Functions_To_Extract();

  if (func_extract_names.size() == 0) {
    std::cout << "Error: No function to extract." << '\n'
              << "note: pass -DCE_EXTRACT_FUNCTIONS=func<1>,...,func<n> to determine "
              << "which functions to extract." << '\n'
              << "note: pass -DCE_EXPORT_SYMBOLS=func<1>,...,func<n> to determine "
              << "which symbols to export." << '\n';

    return 1;
  }

  PassManager().Run_Passes(args);

  return 0;
}
