#pragma once

#include "ArgvParser.hh"
#include "clang/Frontend/ASTUnit.h"

using namespace llvm;
using namespace clang;

class Pass;

/** @brief Pass manager class: run multiple passes
 *
 * This class encapsulates the engine to run a list of passes. The
 * passes list is declared in this class constructor.
 * */
class PassManager {

  public:
    PassManager();
    ~PassManager();

    /** Run all passes.  */
    void Run_Passes(ArgvParser &args);

    /** Context object in which holds the global state of the pass manager.
        It is also used to communicate between the passes.  */
    class Context
    {
      public:
        Context(ArgvParser &args)
          : FuncExtractNames(args.Get_Functions_To_Extract()),
            Externalize(args.Get_Symbols_To_Externalize()),
            OutputFile(args.Get_Output_File()),
            ExternalizationDisabled(args.Is_Externalization_Disabled()),
            ClangArgs(args.Get_Args_To_Clang()),
            PassNum(0)
        {
        }

        /** The Abstract Syntax Tree.  */
        std::unique_ptr<ASTUnit> AST;

        /** The Overlay File System between the real filesystem and the
            in-memory file system.  */
        IntrusiveRefCntPtr<vfs::OverlayFileSystem> OFS;

        /** The in-memory file system used to hold our temporary code.  */
        IntrusiveRefCntPtr<vfs::InMemoryFileSystem> MFS;

        /** List of functions to extract.  */
        std::vector<std::string> &FuncExtractNames;

        /** List of functions to externalize.  */
        std::vector<std::string> &Externalize;

        /** The final output file name.  */
        std::string &OutputFile;

        /** Is the externalization passes disabled?  */
        bool ExternalizationDisabled;

        /** The arguments that will be sent to clang when building the AST.  */
        std::vector<const char *> &ClangArgs;

        /** Current pass number in the passes list.  */
        int PassNum;

        /** Path to input file.  */
        std::string InputPath;

        /** Generated code by the pass.  */
        std::string CodeOutput;
    };

  private:
    /** Pass list.  */
    std::vector<Pass *> Passes;
};

class Pass {
  public:
    /** Default destructor.  */
    virtual ~Pass() {};

    /** Used to determine if pass should run.  Must be implemented by child
        class.  */
    virtual bool Gate(PassManager::Context *) = 0;

    /** What should be done in the pass.  Must be implemented by child class.  */
    virtual bool Run_Pass(PassManager::Context *) = 0;

    /** Dump the pass result to a file.  Must be implemented by child class.  */
    virtual void Dump_Result(PassManager::Context *) = 0;

  friend class PassManager;

  protected:
    /** Determine the output file for the pass dump file.  */
    std::string Get_Dump_Name_From_Input(PassManager::Context *ctx);

    /** Name of current pass.  */
    const char *PassName;
};
