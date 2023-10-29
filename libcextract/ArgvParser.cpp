#include "ArgvParser.hh"
#include "NonLLVMMisc.hh"

ArgvParser::ArgvParser(int argc, char **argv)
  : DisableExternalization(false),
    WithIncludes(false),
    DumpPasses(false)
{
  for (int i = 0; i < argc; i++) {
    if (!Handle_Clang_Extract_Arg(argv[i])) {
      ArgsToClang.push_back(argv[i]);
    }
  }

  Insert_Required_Parameters();
}

void ArgvParser::Insert_Required_Parameters(void)
{
  std::vector<const char *> priv_args = {
     "-fno-builtin", // clang interposes some glibc functions and then it fails to find the declaration of them.
    "-Xclang", "-detailed-preprocessing-record",
    "-Wno-unused-variable", // Passes may instroduce unused variables later removed.
    // For some reason libtooling do not pass the clang include folder.  Pass this then.
    "-I/usr/lib64/clang/" STRINGFY_VALUE(__clang_major__) "/include",
    "-Wno-gnu-variable-sized-type-not-at-end",
  };

  for (const char *arg : priv_args) {
    ArgsToClang.push_back(arg);
  }
}

bool ArgvParser::Handle_Clang_Extract_Arg(const char *str)
{
  // TODO: Review these options from time to time in order to remove them as
  // soon as they are supported by clang
  std::vector<const char *> gcc_args = {
    "-mpreferred-stack-boundary=3",
    "-mindirect-branch=thunk-extern",
    "-mindirect-branch-register",
    "-fno-var-tracking-assignments",
    "-fconserve-stack",
    "-mrecord-mcount",
    "-fconserve-stack",
    "-falign-jumps=1",
    "-fasan-shadow-offset=0xdffffc0000000000",
    "-fno-allow-store-data-races",
    "-Wno-alloc-size-larger-than",
    "-Wno-dangling-pointer",
    "-Wno-format-truncation",
    "-Wno-format-overflow",
    "-Wno-maybe-uninitialized",
    "-Wno-packed-not-aligned",
    "-Wno-restrict",
    "-Wno-stringop-overflow",
    "-Wno-stringop-truncation",
    "-Werror=designated-init",
    "-Wimplicit-fallthrough=5",
  };

  /* Ignore gcc arguments that are not known to clang */
  /* FIXME: Use std::find... */
  for (const char *arg : gcc_args)
  	if (prefix(arg, str))
		return true;

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
  if (prefix("-DCE_EXPAND_INCLUDES=", str)) {
    HeadersToExpand = Extract_Args(str);

    return true;
  }

  return false;
}
