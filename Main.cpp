#include "PrettyPrint.hh"
#include "FunctionDepsFinder.hh"

#include "clang/Tooling/Tooling.h"
#include "clang/Analysis/CallGraph.h"

#include <iostream>

using namespace clang;

static const char *Read_Entire_File(const char *path)
{
  FILE *f = fopen(path, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *string = new char[fsize + 1];
  fread(string, fsize, 1, f);
  fclose(f);

  string[fsize] = '\0';

  return (const char *) string;
}

int main(int argc, char **argv)
{
  if (argc == 3) {
    const char *source_code = Read_Entire_File(argv[1]);

    const std::vector<std::string> args = {
       "--language=c",
       "-I/usr/lib64/clang/13.0.1/include",
       "-I/usr/local/include",
       "-I/usr/bin/../lib64/gcc/x86_64-suse-linux/11/../../../../x86_64-suse-linux/include",
       "-I/usr/include",

       "-I/home/giulianob/projects/libpulp/libpulp/scripts/15-SP4/libopenssl1_1/1.1.1l-150400.7.7.1/src/openssl-1.1.1l/include",
       "-I/home/giulianob/projects/libpulp/libpulp/scripts/15-SP4/libopenssl1_1/1.1.1l-150400.7.7.1/src/openssl-1.1.1l/",
       "-I/home/giulianob/projects/libpulp/libpulp/scripts/15-SP4/libopenssl1_1/1.1.1l-150400.7.7.1/src/openssl-1.1.1l/ssl",
       "-fno-builtin", // clang interposes some glibc functions and then it fails to find the declaration of them.
       "-Xclang", "-detailed-preprocessing-record",
    };

    std::unique_ptr<ASTUnit> ast = clang::tooling::buildASTFromCodeWithArgs
      (source_code, args);
    std::string funcname(argv[2]);

    /* Set SourceManager to PrettyPrinter.  This is important to get Macro
       support working.  */
    PrettyPrint::Set_Source_Manager(&ast->getSourceManager());

    FunctionDependencyFinder(std::move(ast), funcname).Print();

    delete[] (char *) source_code;
  }

  return 0;
}
