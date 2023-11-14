#include "Passes.hh"

#include "FunctionDepsFinder.hh"
#include "ArgvParser.hh"
#include "PrettyPrint.hh"
#include "FunctionExternalizeFinder.hh"
#include "SymbolExternalizer.hh"
#include "ClangCompat.hh"
#include "TopLevelASTIterator.hh"

#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"

#include <iostream>

using namespace llvm;
using namespace clang;

void Print_AST(ASTUnit *ast)
{
  for (auto it = ast->top_level_begin(); it != ast->top_level_end(); ++it) {
    Decl *decl = *it;
    decl->print(llvm::outs(), PrintingPolicy(LangOptions()));
    llvm::outs() << '\n';
  }
}

static bool Build_ASTUnit(PassManager::Context *ctx, IntrusiveRefCntPtr<vfs::FileSystem> fs = nullptr)
{
  ctx->AST.reset();

  IntrusiveRefCntPtr<DiagnosticsEngine> Diags;
  std::shared_ptr<CompilerInvocation> CInvok;
  std::shared_ptr<PCHContainerOperations> PCHContainerOps;

  if (!fs) {
    /* Create a virtual file system.  */
    ctx->OFS = IntrusiveRefCntPtr<vfs::OverlayFileSystem>(new vfs::OverlayFileSystem(vfs::getRealFileSystem()));
    ctx->MFS = IntrusiveRefCntPtr<vfs::InMemoryFileSystem>(new vfs::InMemoryFileSystem);

    /* Push an additional memory filesystem on top of the overlay filesystem
       which will hold temporary modified files.  */
    ctx->OFS->pushOverlay(ctx->MFS);

    fs = ctx->OFS;
  }

  /* Built the ASTUnit from the passed command line and set its SourceManager
     to the PrettyPrint class.  */
  Diags = CompilerInstance::createDiagnostics(new DiagnosticOptions());
  CInvok = ClangCompat::createInvocationFromCommandLine(ctx->ClangArgs, Diags);

  FileManager *FileMgr = new FileManager(FileSystemOptions(), fs);
  PCHContainerOps = std::make_shared<PCHContainerOperations>();

  auto AU = ASTUnit::LoadFromCompilerInvocation(
      CInvok, PCHContainerOps, Diags, FileMgr, false, CaptureDiagsKind::None, 1,
      TU_Complete, false, false, false);

  if (AU == nullptr) {
    std::cerr << "Could not create the AST";
    return false;
  }

  PrettyPrint::Set_Source_Manager(&AU->getSourceManager());
  ctx->AST = std::move(AU);

  return true;
}

std::string Pass::Get_Dump_Name_From_Input(PassManager::Context *ctx)
{
  std::string work = ctx->InputPath;
  int passnum = ctx->PassNum;

  size_t last_dot = work.find_last_of(".");
  std::string no_extension = work.substr(0, last_dot);
  std::string extension = work.substr(last_dot, work.length());

  return no_extension + ".dump." + std::to_string(passnum) +
         "." + std::string(PassName) + extension;
}

/** BuildASTPass: Built the AST object and store it into the Context object.
 *
 * This may be the first pass of the pass queue, as the AST object is used by
 * many foward passes.
 */
class BuildASTPass : public Pass
{
  public:
  BuildASTPass()
  {
    PassName = "BuildASTPass";
  }

  /** Remove any undesired flags and insert new flags as necessary. */
  void Update_Clang_Args(PassManager::Context *ctx)
  {
    bool keep_includes = ctx->KeepIncludes;
    std::vector<const char *> &clangargs = ctx->ClangArgs;

    if (keep_includes == false) {
      /* Linux adds -include header.h flag which we need to remove, else
         the re-inclusion of this header when reparsing will overwrite our
         changes.  */
      for (auto it = clangargs.begin(); it != clangargs.end(); it++) {
        const char *elem = *it;
        if (!strcmp(elem, "-include")) {
          clangargs.erase(it, it+2);

          /* Required because we removed two elements from the vector.  */
          it--;
          it--;
        }
      }
    }
  }

  virtual bool Gate([[maybe_unused]] PassManager::Context *ctx)
  {
    return true;
  }

  virtual bool Run_Pass(PassManager::Context *ctx)
  {
    if (!Build_ASTUnit(ctx))
      return false;

    /* Remove any unwanted arguments from command line.  */
    Update_Clang_Args(ctx);

    /* Get the input file path.  */
    ctx->InputPath = Get_Input_File(ctx->AST.get()).str();

    const DiagnosticsEngine &de = ctx->AST->getDiagnostics();
    return !de.hasErrorOccurred();
  }

  virtual void Dump_Result(PassManager::Context *ctx)
  {
    /* If the code is too large, this crashes with an stack overflow.  */
    return;

    clang::ASTUnit::top_level_iterator it;

    std::error_code ec;
    llvm::raw_fd_ostream out(Get_Dump_Name_From_Input(ctx), ec);

    PrettyPrint::Set_Source_Manager(nullptr);
    PrettyPrint::Set_Output_Ostream(&out);

    for (it = ctx->AST->top_level_begin(); it != ctx->AST->top_level_end(); ++it) {
      Decl *decl = *it;
      PrettyPrint::Print_Decl(decl);
    }

    PrettyPrint::Set_Source_Manager(&ctx->AST->getSourceManager());
    PrettyPrint::Set_Output_Ostream(nullptr);
    out.close();
  }

  StringRef Get_Input_File(ASTUnit *ast)
  {
    SourceManager &sm = ast->getSourceManager();

    const FileEntry *main_file = sm.getFileEntryForID(sm.getMainFileID());
    StringRef path = sm.getFileManager().getCanonicalName(main_file);

    return path;
  }
};

/** ClosurePass: Compute the closure of functions and output then.
 *
 * This pass computes the closure of multiple functions.  That means it add for
 * output every declaration, macro, function, variable that is necessary for
 * the function to be compiled into a separate file.  Check FunctionDepsFinder
 * and MacroDepsFinder classes for more information.
 */
class ClosurePass : public Pass
{
  public:
    ClosurePass(bool print_to_file = false)
      : PrintToFile(print_to_file)
    {
      PassName = "ClosurePass";
    }

  private:
    virtual bool Gate(PassManager::Context *ctx)
    {
      return (!ctx->ExternalizationDisabled || PrintToFile) &&
             ctx->FuncExtractNames.size() > 0;
    }

    std::string Get_Output_From_Input_File(std::string &input)
    {
      std::string work = input;

      size_t last_dot = work.find_last_of(".");
      std::string no_extension = work.substr(0, last_dot);
      std::string extension = work.substr(last_dot, work.length());

      return no_extension + ".CE" + extension;
    }

    virtual bool Run_Pass(PassManager::Context *ctx)
    {
      ctx->CodeOutput = std::string();
      raw_string_ostream code_stream(ctx->CodeOutput);

      PrettyPrint::Set_Output_Ostream(&code_stream);

      /* Compute closure and output the code.  */
      FunctionDependencyFinder fdf(ctx);
      fdf.Print();

      /* Add the temporary string with code to the filesystem.  */
      ctx->MFS->addFile(ctx->InputPath, 0, MemoryBuffer::getMemBufferCopy(ctx->CodeOutput));

      //Print_AST(ctx->AST.get());

      /* Parse the temporary code to apply the changes by the externalizer
         and set its new SourceManager to the PrettyPrint class.  In case we
         must keep the includes, pass the overlayFS instead of the memoryFS
         as we must access the headers in disk.  If we are expanding all
         headers, pass the memoryFS so we can catch possible bugs where
         #includes went through.  */
      if (ctx->KeepIncludes) {
        if (!Build_ASTUnit(ctx, ctx->OFS)) {
          return false;
        }
      } else {
        if (!Build_ASTUnit(ctx, ctx->MFS)) {
          return false;
        }
      }

      /* If there was an error on building the AST here, don't continue.  */
      const DiagnosticsEngine &de = ctx->AST->getDiagnostics();
      if (de.hasErrorOccurred()) {
        return false;
      }

      /* Set output stream to a file if we set to print to a file.  */
      if (PrintToFile) {
        std::string output_path = ctx->OutputFile;
        if (output_path == "") {
          output_path = Get_Output_From_Input_File(ctx->InputPath);
        }
        PrettyPrint::Set_Output_To(output_path);
      } else {
        ctx->CodeOutput = std::string();
        PrettyPrint::Set_Output_Ostream(&code_stream);
      }

      /* Compute closure and output the code.  */
      FunctionDependencyFinder fdf2(ctx);
      fdf2.Print();

      /* Add the temporary string with code to the filesystem.  */
      ctx->MFS->addFile(ctx->InputPath, 0, MemoryBuffer::getMemBufferCopy(ctx->CodeOutput));

      const DiagnosticsEngine &de2 = ctx->AST->getDiagnostics();
      return !de2.hasErrorOccurred();
    }

    virtual void Dump_Result(PassManager::Context *ctx)
    {
      std::error_code ec;
      llvm::raw_fd_ostream out(Get_Dump_Name_From_Input(ctx), ec);
      out << ctx->CodeOutput;
      out.close();
    }

    bool PrintToFile;
};

/** FunctionExternalizeFidnerPass: Find which functions and variables should
 *  be externalized.
 *
 * In case the target binary to be livepatched still have references to private
 * symbols, we want to reference them in order to reduce the livepatch size.
 */
class FunctionExternalizeFinderPass : public Pass
{
  public:
    FunctionExternalizeFinderPass()
    {
      PassName = "FunctionExternalizeFinderPass";
    }

  private:
    virtual bool Gate(PassManager::Context *ctx)
    {
      return !ctx->ExternalizationDisabled && ctx->Externalize.size() == 0;
    }

    virtual bool Run_Pass(PassManager::Context *ctx)
    {
      /* Find which symbols must be externalized.  */
      FunctionExternalizeFinder fef(ctx->AST.get(),
          ctx->FuncExtractNames,
          ctx->Externalize,
          ctx->KeepIncludes,
          ctx->IA);
      ctx->Externalize = fef.Get_To_Externalize();

      return true;
    }

    virtual void Dump_Result(PassManager::Context *ctx)
    {
      std::error_code ec;
      llvm::raw_fd_ostream out(Get_Dump_Name_From_Input(ctx), ec);
      out << "Functions to Externalize:\n";

      for (const std::string &str : ctx->Externalize) {
        out << str << '\n';
      }
    }
};

/** FunctionExternalizerPass: Externalize given symbols.
 *
 * Check SymbolExternalizer class for more information.
 */
class FunctionExternalizerPass : public Pass
{
  public:
    FunctionExternalizerPass()
    {
      PassName = "FunctionExternalizerPass";
    }

  private:
    virtual bool Gate(PassManager::Context *ctx)
    {
      return ctx->Externalize.size() > 0;
    }

    virtual bool Run_Pass(PassManager::Context *ctx)
    {
      /* Issue externalization.  */
      SymbolExternalizer externalizer(ctx->AST.get(), ctx->IA);
      externalizer.Externalize_Symbols(ctx->Externalize);
      externalizer.Commit_Changes_To_Source(ctx->OFS, ctx->MFS, ctx->HeadersToExpand);

      if (ctx->DumpPasses) {
        /* Something for the poor debugging user.  */
        ctx->CodeOutput = externalizer.Get_Modifications_To_Main_File();
      }

      /* Parse the temporary code to apply the changes by the externalizer
         and set its new SourceManager to the PrettyPrint class.  */
      ctx->AST->Reparse(std::make_shared<PCHContainerOperations>(), ClangCompat_None, ctx->OFS);
      PrettyPrint::Set_Source_Manager(&ctx->AST->getSourceManager());

      const DiagnosticsEngine &de = ctx->AST->getDiagnostics();
      return !de.hasErrorOccurred();
    }

    virtual void Dump_Result(PassManager::Context *ctx)
    {
      std::error_code ec;
      llvm::raw_fd_ostream out(Get_Dump_Name_From_Input(ctx), ec);
      /* Dump the filesystem first.  */
      out << "/*  LLVM in-memory virtual filesystem.\n";
      ctx->MFS->print(out);
      out << "*/\n";

      /* The list of headers that should be expanded.  */
      out << "/*  Headers to expand:\n";
      for (const std::string &h : ctx->HeadersToExpand) {
        out << "  " << h << '\n';
      }
      out << "*/\n";

      /* Then the code.  */
      out << ctx->CodeOutput;
    }
};

PassManager::PassManager()
{
  /* Declare the pass list.  Passes will run in this order.  */
  Passes = {
    new BuildASTPass(),
    new ClosurePass(/*PrintToFile=*/false),
    new FunctionExternalizeFinderPass(),
    new FunctionExternalizerPass(),
    new ClosurePass(/*PrintToFile=*/true),
  };
}

PassManager::~PassManager()
{
  /* Delete pass list.  */
  for (Pass *pass : Passes) {
    delete pass;
  }
}

void PassManager::Run_Passes(ArgvParser &args)
{
  /* Build context object to avoid using global variables.  */
  Context ctx(args);

  /* Run the pass list.  */
  for (Pass *pass : Passes) {
    ctx.PassNum++;
    if (pass->Gate(&ctx)) {
      bool pass_success = pass->Run_Pass(&ctx);

      if (ctx.DumpPasses) {
        pass->Dump_Result(&ctx);
      }

      if (pass_success == false) {
        std::cerr << '\n' << "Error on pass: " << pass->PassName << '\n';
        return;
      }
    }
  }
}
