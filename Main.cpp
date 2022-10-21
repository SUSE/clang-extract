#include "PrettyPrint.hh"
#include "FunctionDepsFinder.hh"
#include "MacroDepsFinder.hh"
#include "SymbolExternalizer.hh"
#include "ArgvParser.hh"
#include "FunctionExternalizeFinder.hh"

#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"

#include <iostream>

using namespace llvm;
using namespace clang;

static std::unique_ptr<ASTUnit> Create_ASTUnit(const ArgvParser &args, IntrusiveRefCntPtr<vfs::FileSystem> vfs = vfs::getRealFileSystem())
{
  IntrusiveRefCntPtr<DiagnosticsEngine> Diags;
  std::shared_ptr<CompilerInvocation> CInvok;
  std::shared_ptr<PCHContainerOperations> PCHContainerOps;
  const std::vector<const char *> &args_vector = args.Get_Args_To_Clang();

  Diags = CompilerInstance::createDiagnostics(new DiagnosticOptions());
  CreateInvocationOptions CIOpts;
  CIOpts.Diags = Diags;
  CInvok = createInvocation(args_vector, std::move(CIOpts));

  FileManager *FileMgr = new FileManager(FileSystemOptions(), vfs);
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

static void Closure_And_Print(ASTUnit *ast, std::vector<std::string> const &extract_names, bool macros_enabled)
{
  if (macros_enabled) {
    MacroDependencyFinder(ast, extract_names).Print();
  } else {
    PrettyPrint::Set_Source_Manager(nullptr);
    FunctionDependencyFinder(ast, extract_names).Print();
  }
}

void Free_String_Pool(void);

int main(int argc, char **argv)
{
  ArgvParser Args(argc, argv);

  auto func_extract_names = Args.Get_Functions_To_Extract();
  auto syms_force_externalize = Args.Get_Symbols_To_Externalize();
  bool should_externalize = !Args.Is_Externalization_Disabled();

  if (func_extract_names.size() == 0) {
    std::cout << "Error: No function to extract." << '\n'
              << "note: pass -DCE_EXTRACT_FUNCTIONS=func<1>,...,func<n> to determine "
              << "which functions to extract." << '\n'
              << "note: pass -DCE_EXPORT_SYMBOLS=func<1>,...,func<n> to determine "
              << "which symbols to export." << '\n';

    return 1;
  }

  /* Create a virtual file system.  */
  IntrusiveRefCntPtr<vfs::OverlayFileSystem> ofs(new vfs::OverlayFileSystem(vfs::getRealFileSystem()));
  IntrusiveRefCntPtr<vfs::InMemoryFileSystem> mfs(new vfs::InMemoryFileSystem);

  /* Push an additional memory filesystem on top of the overlay filesystem
     which will hold temporary modified files.  */
  ofs->pushOverlay(mfs);

  /* Built the ASTUnit from the passed command line and set its SourceManager
     to the PrettyPrint class.  */
  std::unique_ptr<ASTUnit> ast = Create_ASTUnit(Args, ofs);
  PrettyPrint::Set_Source_Manager(&ast->getSourceManager());

  /* Get the input file path.  */
  std::string input_path = Get_Input_File(ast->getSourceManager()).str();

  if (should_externalize) {
    /* Create a stream to a string in which we will output temporary code to
       it.  */
    std::string extracted_code;
    llvm::raw_string_ostream code_stream(extracted_code);

    /* Set output stream to string.  */
    PrettyPrint::Set_Output_Ostream(&code_stream);

    /* First step: find which symbols we need by computing the closure and
       printing to a temporary string.  */
    Closure_And_Print(ast.get(), func_extract_names, true);

    /* Add the temporary string with code to the filesystem.  */
    mfs->addFile(input_path, 0, MemoryBuffer::getMemBufferCopy(extracted_code));

    /* Parse the temporary code and set its new SourceManager to the
       PrettyPrint class.  */
    ast->Reparse(std::make_shared<PCHContainerOperations>(), None, mfs);
    PrettyPrint::Set_Source_Manager(&ast->getSourceManager());

    /* Find which symbols must be externalized.  */
    FunctionExternalizeFinder fef(ast.get(), func_extract_names, syms_force_externalize);
    auto syms_externalize = fef.Get_To_Externalize();

    /* If there is something to externalize, then:  */
    if (syms_externalize.size() > 0) {

      /* Isse externalization.  */
      SymbolExternalizer externalizer(ast.get());
      externalizer.Externalize_Symbols(syms_externalize);

      /* Create a new Filesystem because we can't delete the previous temp file
         from the filesystem.  */
      IntrusiveRefCntPtr<vfs::OverlayFileSystem> ofs(new vfs::OverlayFileSystem(vfs::getRealFileSystem()));
      IntrusiveRefCntPtr<vfs::InMemoryFileSystem> mfs(new llvm::vfs::InMemoryFileSystem);
      ofs->pushOverlay(mfs);

      /* Get the new source code and add it to the filesystem.  */
      std::string modified = externalizer.Get_Modifications_To_Main_File();
      llvm::outs() << modified << "\n\n\n";

      mfs->addFile(input_path, 0, MemoryBuffer::getMemBufferCopy(modified));

      /* Parse the temporary code to apply the changes by the externalizer
         and set its new SourceManager to the PrettyPrint class.  */
      ast->Reparse(std::make_shared<PCHContainerOperations>(), None, mfs);
      PrettyPrint::Set_Source_Manager(&ast->getSourceManager());
    }
  }

  /* Set output file.  */
  std::string output_path = Args.Get_Output_File();
  if (output_path == "") {
    output_path = Get_Output_From_Input_File(ast->getSourceManager());
  }
  PrettyPrint::Set_Output_To(output_path);

  /* Final pass: compute closure and print result.  */
  Closure_And_Print(ast.get(), func_extract_names, true);

  Free_String_Pool();
  return 0;
}
