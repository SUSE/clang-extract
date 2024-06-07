//===- ArgvParser.cpp - Parse the command-line options ----------*- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Parse the command-line options given to clang-extract.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#include "ArgvParser.hh"
#include "NonLLVMMisc.hh"
#include "Error.hh"

#include <clang/Basic/Version.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
/* Use the basename version that doesn't change the input string */
#include <string.h>

#ifndef CLANG_VERSION_MAJOR
# error "Unable to find clang version"
#endif

/** List of arguments that are supported by GCC but unsupported by clang,
    resulting in compilation error.  */
static const char *UnsupportedGCCArgs[] = {
  "-mpreferred-stack-boundary=",
  "-mindirect-branch=thunk-extern",
  "-mindirect-branch-register",
  "-mrecord-mcount",
  "-falign-jumps=",
  "-fasan-shadow-offset=",
  "-fconserve-stack",
  "-fdump-ipa-clones",
  "-flive-patching=inline-clone",
  "-fmin-function-alignment=16",
  "-fno-allow-store-data-races",
  "-fno-var-tracking-assignments",
  "-Wno-alloc-size-larger-than",
  "-Wno-dangling-pointer",
  "-Wno-format-truncation",
  "-Wno-format-overflow",
  "-Wmaybe-uninitialized",
  "-Wno-maybe-uninitialized",
  "-Wpacked-not-aligned",
  "-Wno-packed-not-aligned",
  "-Wno-restrict",
  "-Wno-stringop-overflow",
  "-Wstringop-truncation",
  "-Wno-stringop-truncation",
  "-Werror=designated-init",
  "-Wimplicit-fallthrough=",
};

ArgvParser::ArgvParser(int argc, char **argv)
  : ArgsToClang(),
    FunctionsToExtract(),
    SymbolsToExternalize(),
    HeadersToExpand(),
    OutputFile(),
    IgnoreClangErrors(false),
    DisableExternalization(false),
    WithIncludes(false),
    DumpPasses(false),
    RenameSymbols(false),
    Kernel(false),
    Ibt(false),
    AllowLateExternalization(false),
    PatchObject(""),
    Debuginfos(),
    IpaclonesPath(nullptr),
    SymversPath(nullptr),
    DescOutputPath(nullptr),
    IncExpansionPolicy(nullptr),
    OutputFunctionPrototypeHeader(nullptr)
{
  for (int i = 0; i < argc; i++) {
    if (!Handle_Clang_Extract_Arg(argv[i])) {
      ArgsToClang.push_back(argv[i]);
    }
  }

  Insert_Required_Parameters();

  const char *DebuginfoPath = nullptr;
  if (Debuginfos.size() > 0) {
    DebuginfoPath = Debuginfos[0].c_str();
  }

  /* For kernel, check if the object patch is not the same as DebugInfo. If they
   * are not the same, it means that the module from PatchObject is builtin, so
   * assign vmlinux to PatchObject. */
  if (Kernel && DebuginfoPath) {
    std::string obj_path = basename(DebuginfoPath);
    /* As the DebugInfo can point to a file with suffix (btrfs.ko for example),
     * check the substring */
    if (obj_path.find(PatchObject) == std::string::npos)
      PatchObject = "vmlinux";
  }
}

void ArgvParser::Insert_Required_Parameters(void)
{
  std::vector<const char *> priv_args = {
    "-Xclang", "-detailed-preprocessing-record",
    // For some reason libtooling do not pass the clang include folder.  Pass this then.
#ifdef __i386__
    "-I/usr/lib/clang/" STRINGFY_VALUE(CLANG_VERSION_MAJOR) "/include",
#else
    "-I/usr/lib64/clang/" STRINGFY_VALUE(CLANG_VERSION_MAJOR) "/include",
#endif
    "-Wno-gnu-variable-sized-type-not-at-end",
    "-Wno-incompatible-pointer-types-discards-qualifiers", // this can be triggered for older codestreams
    "-Wno-missing-prototypes", // We remove the static keyword from the
                               // extracted function, so disable warnings
                               // related to it.
    "-Wno-unused-function", // May happen when trying to extract a static function.
    "-Wno-unused-variable", // Passes may instroduce unused variables later removed.
    "-fno-builtin", // clang interposes some glibc functions and then it fails to find the declaration of them.
    "-Wno-duplicate-decl-specifier", // Disabled due to kernel issues. See more
                                     // at https://github.com/ClangBuiltLinux/linux/issues/2013
                                     // and https://github.com/llvm/llvm-project/issues/93449
  };

  for (const char *arg : priv_args) {
    ArgsToClang.push_back(arg);
  }
}

void ArgvParser::Print_Usage_Message(void)
{
  llvm::outs() <<
"OVERVIEW: Tool to extract code from C projects using the LLVM infrastructure.\n"
"          It should be invoked as a C compiler.\n"
"\n"
"USAGE: clang-extract [options] file...\n"
"\n"
"CLANG-EXTRACT OPTIONS:\n"
"   <clang-switch>          A clang switch, as specified by calling clang --help.\n"
"  -D__KERNEL__             Indicate that we are processing a Linux sourcefile.\n"
"  -DCE_EXTRACT_FUNCTIONS=<args>\n"
"                           Extract the functions specified in the <args> list.\n"
"  -DCE_EXPORT_SYMBOLS=<args>\n"
"                           Force externalization of symbols specified in the <args> list\n"
"  -DCE_OUTPUT_FILE=<arg>   Output code to <arg> file.  Default is <input>.CE.c.\n"
"  -DCE_NO_EXTERNALIZATION  Disable symbol externalization.\n"
"  -DCE_DUMP_PASSES         Dump the results of each transformation pass into files.\n"
"  -DCE_KEEP_INCLUDES       Keep all possible #include<file> directives.\n"
"  -DCE_KEEP_INCLUDES=<policy>\n"
"                           Keep all possible #include<file> directives, but using the\n"
"                           specified include expansion <policy>.  Valid values are\n"
"                           nothing, everything and kernel.\n"
"  -DCE_EXPAND_INCLUDES=<args>\n"
"                           Force expansion of the headers provided in <args>.\n"
"  -DCE_RENAME_SYMBOLS      Allow renaming of extracted symbols.\n"
"  -DCE_DEBUGINFO_PATH=<arg>\n"
"                           Path to the compiled (ELF) object of the desired program to\n"
"                           extract.  This is used to decide if externalization is\n"
"                           necessary or not for given symbol.\n"
"  -DCE_IPACLONES_PATH=<arg>\n"
"                           Path to .ipa-clones files generated by gcc.  Used to decide\n"
"                           if desired function to extract was inlined into other\n"
"                           functions.\n"
"  -DCE_SYMVERS_PATH=<arg>  Path to kernel Modules.symvers file.  Only used when\n"
"                           -D__KERNEL__ is specified.\n"
"  -DCE_DSC_OUTPUT=<arg>    Libpulp .dsc file output, used for userspace livepatching.\n"
"  -DCE_OUTPUT_FUNCTION_PROTOTYPE_HEADER=<arg>\n"
"                           Outputs a header file with a foward declaration of all\n"
"                           functions. This header is not self-compilable.\n"
"  -DCE_LATE_EXTERNALIZE    Enable late externalization (declare externalized variables\n"
"                           later than the original).  May reduce code output when\n"
"                           -DCE_KEEP_INCLUDES is enabled\n"
"  -DCE_IGNORE_CLANG_ERRORS Ignore clang compilation errors in a hope that code is\n"
"                           generated even if it won't compile.\n"
"\n";

  llvm::outs() << "The following arguments are ignored by clang-extract:\n";
  for (unsigned long i = 0; i < ARRAY_LENGTH(UnsupportedGCCArgs); i++) {
    llvm::outs() << "  " << UnsupportedGCCArgs[i] << '\n';
  }

  llvm::outs() << '\n';
  llvm::outs() << "For a list of switches accepted by clang, run:\n" <<
    "  $ clang --help\n";
}

bool ArgvParser::Handle_Clang_Extract_Arg(const char *str)
{
  /* Ignore gcc arguments that are not known to clang */
  for (unsigned long i = 0; i < ARRAY_LENGTH(UnsupportedGCCArgs); i++) {
    if (prefix(UnsupportedGCCArgs[i], str)) {
      return true;
    }
  }

  if (!strncmp("-D__KERNEL__", str, 12)) {
    Kernel = true;
    return false;
  }

  if (!strcmp("-D__USE_IBT__", str)) {
    Ibt = true;
    return false;
  }

  if (prefix("-DKBUILD_MODNAME=", str)) {
    /* Avoid storing double quotes */
    PatchObject = Extract_Single_Arg(str);
    PatchObject.erase(std::remove(PatchObject.begin(), PatchObject.end(), '\"' ),
                                    PatchObject.end());

    return false;
  }

  if (prefix("-DCE_EXTRACT_FUNCTIONS=", str)) {
    FunctionsToExtract = Extract_Args(str);

    return true;
  }
  if (prefix("-DCE_EXPORT_SYMBOLS=", str)) {
    SymbolsToExternalize = Extract_Args(str);

    return true;
  }
  if (prefix("-DCE_OUTPUT_FILE=", str)) {
    OutputFile = Extract_Single_Arg(str);

    return true;
  }
  if (!strcmp("-DCE_NO_EXTERNALIZATION", str)) {
    DisableExternalization = true;

    return true;
  }
  if (!strcmp("-DCE_DUMP_PASSES", str)) {
    DumpPasses = true;

    return true;
  }
  if (!strcmp("-DCE_KEEP_INCLUDES", str)) {
    WithIncludes = true;
    return true;
  }
  if (prefix("-DCE_KEEP_INCLUDES=", str)) {
    WithIncludes = true;
    IncExpansionPolicy = Extract_Single_Arg_C(str);
    return true;
  }
  if (prefix("-DCE_EXPAND_INCLUDES=", str)) {
    HeadersToExpand = Extract_Args(str);

    return true;
  }
  if (prefix("-DCE_DEBUGINFO_PATH=", str)) {
    Debuginfos = Extract_Args(str);

    return true;
  }
  if (prefix("-DCE_IPACLONES_PATH=", str)) {
    IpaclonesPath = Extract_Single_Arg_C(str);

    return true;
  }
  if (prefix("-DCE_SYMVERS_PATH=", str)) {
    SymversPath = Extract_Single_Arg_C(str);

    return true;
  }
  if (prefix("-DCE_DSC_OUTPUT=", str)) {
    DescOutputPath = Extract_Single_Arg_C(str);

    return true;
  }
  if (prefix("-DCE_OUTPUT_FUNCTION_PROTOTYPE_HEADER=", str)) {
    OutputFunctionPrototypeHeader = Extract_Single_Arg_C(str);

    return true;
  }
  if (!strcmp("-DCE_RENAME_SYMBOLS", str)) {
    RenameSymbols = true;

    return true;
  }
  if (!strcmp("-DCE_LATE_EXTERNALIZE", str)) {
    AllowLateExternalization = true;

    return true;
  }
  if (!strcmp("-DCE_IGNORE_CLANG_ERRORS", str)) {
    IgnoreClangErrors = true;

    return true;
  }

  if (!strcmp("--help", str)) {
    Print_Usage_Message();
    exit(0);
  }
  if (prefix("-DCE_", str)) {
    DiagsClass::Emit_Error("Unrecognized command-line option: " + std::string(str));
    exit(1);
  }

  return false;
}
