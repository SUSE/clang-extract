//===- Passes.hh - Pass manager and pass list for clang-extract -*- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Pass manager and pass list for clang-extract.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#pragma once

#include "ArgvParser.hh"
#include "InlineAnalysis.hh"
#include "SymbolExternalizer.hh"
#include "ExpansionPolicy.hh"
#include "clang/Frontend/ASTUnit.h"

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
    int Run_Passes(ArgvParser &args);

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
            KeepIncludes(args.Should_Keep_Includes()),
            DumpPasses(args.Should_Dump_Passes()),
            RenameSymbols(args.Should_Rename_Symbols()),
            Kernel(args.Is_Kernel()),
            HeadersToExpand(args.Get_Headers_To_Expand()),
            ClangArgs(args.Get_Args_To_Clang()),
            DebuginfoPath(args.Get_Debuginfo_Path()),
            IpaclonesPath(args.Get_Ipaclones_Path()),
            SymversPath(args.Get_Symvers_Path()),
            DscOutputPath(args.Get_Dsc_Output_Path()),
            OutputFunctionPrototypeHeader(args.Get_Output_Path_To_Prototype_Header()),
            IncExpansionPolicy(IncludeExpansionPolicy::Get_Overriding(
                               args.Get_Include_Expansion_Policy(), Kernel)),
            NamesLog(),
            PassNum(0),
            IA(DebuginfoPath, IpaclonesPath, SymversPath, args.Is_Kernel())
        {
        }

        /** The Abstract Syntax Tree.  */
        std::unique_ptr<ASTUnit> AST;

        /** The Overlay File System between the real filesystem and the
            in-memory file system.  */
        IntrusiveRefCntPtr<llvm::vfs::OverlayFileSystem> OFS;

        /** The in-memory file system used to hold our temporary code.  */
        IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> MFS;

        /** List of functions to extract.  */
        std::vector<std::string> &FuncExtractNames;

        /** List of functions to externalize.  */
        std::vector<std::string> &Externalize;

        /** The final output file name.  */
        std::string &OutputFile;

        /** Is the externalization passes disabled?  */
        bool ExternalizationDisabled;

        /** Should we keep the includes whenever is possible?  */
        bool KeepIncludes;

        /** Should the passes dump something?  */
        bool DumpPasses;

        /** Should symbols marked for extraction be renamed?  */
        bool RenameSymbols;

        /** If the source code comes from Linux Kernel */
        bool Kernel;

        /** Which includes we must expand? */
        std::vector<std::string> HeadersToExpand;

        /** The arguments that will be sent to clang when building the AST.  */
        std::vector<const char *> &ClangArgs;

        /* Path to Debuginfo, if exists.  */
        const char *DebuginfoPath;

        /* Path to Ipaclones, if exists.  */
        const char *IpaclonesPath;

        /* Path to Symvers, if exists.  */
        const char *SymversPath;

        /* Path to libpulp .dsc file for output.  */
        const char *DscOutputPath;

        /* Output path to a file containing foward declarations of all functions.  */
        const char *OutputFunctionPrototypeHeader;

        /* Policy used to expand includes.  */
        IncludeExpansionPolicy::Policy IncExpansionPolicy;

        /** Log of changed names.  */
        std::vector<ExternalizerLogEntry> NamesLog;

        /** Current pass number in the passes list.  */
        int PassNum;

        /** Path to input file.  */
        std::string InputPath;

        /** Generated code by the pass.  */
        std::string CodeOutput;

        /* InlineAnalysis object that will persists through the entire analysis.
           Avoid rebuilding it as it may require parsing several very large
           files, thus becoming very slow.  */
        InlineAnalysis IA;
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
