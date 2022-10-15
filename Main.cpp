#include "PrettyPrint.hh"
#include "FunctionDepsFinder.hh"
#include "MacroDepsFinder.hh"
#include "FunctionExternalizer.hh"
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
    FunctionExternalizer externalizer(ast.get());
    externalizer.Externalize_Symbols(syms_externalize);

    ast->Reparse(std::make_shared< PCHContainerOperations >(), None, nullptr, /*KeepFileMgr=*/true);
    PrettyPrint::Set_Source_Manager(&ast->getSourceManager());
  }

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
