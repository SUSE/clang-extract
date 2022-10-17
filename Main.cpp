#include "PrettyPrint.hh"
#include "FunctionDepsFinder.hh"
#include "MacroDepsFinder.hh"
#include "SymbolExternalizer.hh"
#include "ArgvParser.hh"

#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"

#include <iostream>

using namespace llvm;
using namespace clang;

static std::unique_ptr<ASTUnit> Create_ASTUnit(const ArgvParser &args)
{
  IntrusiveRefCntPtr<DiagnosticsEngine> Diags;
  std::shared_ptr<CompilerInvocation> CInvok;
  std::shared_ptr<PCHContainerOperations> PCHContainerOps;
  const std::vector<const char *> &args_vector = args.Get_Args_To_Clang();

  Diags = CompilerInstance::createDiagnostics(new DiagnosticOptions());
  CreateInvocationOptions CIOpts;
  CIOpts.Diags = Diags;
  CInvok = createInvocation(args_vector, std::move(CIOpts));

  FileManager *FileMgr = new FileManager(FileSystemOptions(), vfs::getRealFileSystem());
  PCHContainerOps = std::make_shared<PCHContainerOperations>();

  auto AU = ASTUnit::LoadFromCompilerInvocation(
      CInvok, PCHContainerOps, Diags, FileMgr, false, CaptureDiagsKind::None, 1,
      TU_Complete, false, false, false);

  return AU;
}

static StringRef Get_Input_File(const SourceManager &sm)
{
  const FileEntry *main_file = sm.getFileEntryForID(sm.getMainFileID());
  StringRef path = sm.getFileManager().getCanonicalName(main_file);

  return path;
}

static std::string Get_Output_From_Input_File(const SourceManager &sm)
{
  StringRef input = Get_Input_File(sm);
  std::string work = input.str();

  size_t last_dot = work.find_last_of(".");
  std::string no_extension = work.substr(0, last_dot);
  std::string extension = work.substr(last_dot, work.length());

  return no_extension + ".CE" + extension;
}

void Free_String_Pool(void);

int main(int argc, char **argv)
{
  ArgvParser Args(argc, argv);

  std::unique_ptr<ASTUnit> ast = Create_ASTUnit(Args);

  auto func_extract_names = Args.Get_Functions_To_Extract();
  auto syms_externalize = Args.Get_Symbols_To_Externalize();

  if (func_extract_names.size() == 0) {
    std::cout << "Error: No function to extract." << '\n'
              << "note: pass -DCE_EXTRACT_FUNCTIONS=func<1>,...,func<n> to determine "
              << "which functions to extract." << '\n'
              << "note: pass -DCE_EXPORT_SYMBOLS=func<1>,...,func<n> to determine "
              << "which symbols to export." << '\n';

    return 1;
  }

  PrettyPrint::Set_Source_Manager(&ast->getSourceManager());

  if (syms_externalize.size() > 0) {
    SymbolExternalizer externalizer(ast.get());
    externalizer.Externalize_Symbols(syms_externalize);

    ast->Reparse(std::make_shared< PCHContainerOperations >(), None, nullptr, /*KeepFileMgr=*/true);
    PrettyPrint::Set_Source_Manager(&ast->getSourceManager());
  }

  std::string output_path = Args.Get_Output_File();
  if (output_path == "") {
    output_path = Get_Output_From_Input_File(ast->getSourceManager());
  }

  PrettyPrint::Set_Output_To(output_path);

  const bool macros_enabled = true;
  if (macros_enabled) {
    MacroDependencyFinder(ast.get(), func_extract_names).Print();
  } else {
    PrettyPrint::Set_Source_Manager(nullptr);
    FunctionDependencyFinder(ast.get(), func_extract_names).Print();
  }

  Free_String_Pool();
  return 0;
}
