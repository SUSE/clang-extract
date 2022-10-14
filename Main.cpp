#include "PrettyPrint.hh"
#include "FunctionDepsFinder.hh"
#include "MacroDepsFinder.hh"
#include "FunctionExternalizer.hh"

#include "clang/Tooling/Tooling.h"

#include <iostream>

using namespace clang;

static const char *Read_Entire_File(const char *path)
{
  FILE *f = fopen(path, "rb");
  if (!f) {
    std::cout << "Error: unable to open " << path << std::endl;
    abort();
  }

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *string = new char[fsize + 1];
  fread(string, fsize, 1, f);
  fclose(f);

  string[fsize] = '\0';

  return (const char *) string;
}

void Free_String_Pool(void);

int main(int argc, char **argv)
{
  if (argc < 3) {
    std::cout << "clang-extract file.c function-to-compute-closure [function-to-externalize]";
    return 1;
  }

  const char *source_code = Read_Entire_File(argv[1]);

  const std::vector<std::string> args = {
     "--language=c",
     "-I/usr/lib64/clang/15.0.2/include/",
     "-I/usr/local/include",
     "-I/usr/bin/../lib64/gcc/x86_64-suse-linux/11/../../../../x86_64-suse-linux/include",
     "-I/usr/include",

     "-I/home/giulianob/projects/libpulp/libpulp/scripts/15-SP4/libopenssl1_1/1.1.1l-150400.7.7.1/src/openssl-1.1.1l/include",
     "-I/home/giulianob/projects/libpulp/libpulp/scripts/15-SP4/libopenssl1_1/1.1.1l-150400.7.7.1/src/openssl-1.1.1l/",
     "-I/home/giulianob/projects/libpulp/libpulp/scripts/15-SP4/libopenssl1_1/1.1.1l-150400.7.7.1/src/openssl-1.1.1l/ssl",
     "-fno-builtin", // clang interposes some glibc functions and then it fails to find the declaration of them.
     "-Xclang", "-detailed-preprocessing-record",
     "-Xclang", "-ast-dump"
  };

  std::unique_ptr<ASTUnit> ast = clang::tooling::buildASTFromCodeWithArgs
    (source_code, args);
  std::string funcname(argv[2]);
  std::string func_externalize_name("");

  PrettyPrint::Set_Source_Manager(&ast->getSourceManager());

  if (argc == 4) {
    func_externalize_name = std::string(argv[3]);
  }

  if (func_externalize_name != "") {
    FunctionExternalizer externalizer(ast.get());
    externalizer.Externalize_Symbol(func_externalize_name);

    llvm::vfs::FileSystem &fs = ast->getFileManager().getVirtualFileSystem();
    auto shared_fs = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>(&fs);

    ast->Reparse(std::make_shared< PCHContainerOperations >(), None, shared_fs, /*KeepFileMgr=*/true);
    //ast->Reparse(std::make_shared< PCHContainerOperations >(), None, shared_fs);
    PrettyPrint::Set_Source_Manager(&ast->getSourceManager());
  }

  const bool macros_enabled = true;
  if (macros_enabled) {
    MacroDependencyFinder(ast.get(), funcname).Print();
  } else {
    PrettyPrint::Set_Source_Manager(nullptr);
    FunctionDependencyFinder(ast.get(), funcname).Print();
  }

  delete[] (char *) source_code;
  Free_String_Pool();
  return 0;
}
