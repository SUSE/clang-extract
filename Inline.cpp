#include "InlineAnalysis.hh"
#include <stdlib.h>
#include <stdio.h>
#include <vector>

enum MODE {
  LIST_ALL,
  WHERE_IS_INLINED,
  INLINE_CLOSURE,
};

enum OUTPUT_MODE {
  TERMINAL,
  CSV,
  DOT,
};

static enum OUTPUT_MODE Output = TERMINAL;
static enum MODE Mode = LIST_ALL;

const char *Output_Path = nullptr;

static const char *Elf_Path = nullptr;
static const char *Ipa_Path = nullptr;
static const char *Symvers_Path = nullptr;

static std::vector<std::string> Symbols_To_Analyze;

static void Print_Usage(void)
{
  printf(
" Usage: inline <ARGS> <SYMBOLS>\n"
"   where <ARGS>:\n"
"     -ipa-files <PATH>        Path to the .ipa-clone file,\n"
"     -debuginfo <PATH>        Path to the debuginfo file,\n"
"     -symvers   <PATH>        Path to the Kernel Module.symvers file,\n"
"     -graphviz  <PATH>        Output into <PATH> into a .dot graphviz format,\n"
"     -csv       <PATH>        Output into <path> into a .csv table format,\n"
"     -where-is-inlined        Find where <SYMBOLS> got inlined,\n"
"     -compute-closure         Find symbols that got inlined into <SYMBOLS>\n"
  );
  exit(0);
}

static void Parse(int argc, char *argv[])
{
  if (argc <= 1) {
    Print_Usage();
  }

  for (int i = 1; i < argc; i++) {
    if (i + 1 < argc) {
      if (strcmp(argv[i], "-o") == 0) {
        Output_Path = argv[++i];
        continue;
      }

      if (strcmp(argv[i], "-ipa-files") == 0) {
        Ipa_Path = argv[++i];
        continue;
      }

      if (strcmp(argv[i], "-debuginfo") == 0) {
        Elf_Path = argv[++i];
        continue;
      }

      if (strcmp(argv[i], "-symvers") == 0) {
        Symvers_Path = argv[++i];
        continue;
      }

    }

    if (strcmp(argv[i], "-graphviz") == 0) {
      Output = DOT;
      continue;
    }

    if (strcmp(argv[i], "-csv") == 0) {
      Output = CSV;
      continue;
    }

    if (strcmp(argv[i], "-where-is-inlined") == 0) {
      Mode = WHERE_IS_INLINED;
      continue;
    }

    if (strcmp(argv[i], "-compute-closure") == 0) {
      Mode = INLINE_CLOSURE;
      continue;
    }

    Symbols_To_Analyze.push_back(std::string(argv[i]));
  }
}

static void Check_Input(void)
{
  if (Ipa_Path == nullptr) {
    printf("WARNING: No IPA files found.\n");
  }

  if (Elf_Path == nullptr) {
    printf("WARNING: No debuginfo file found.\n");
  }

  if (Symvers_Path == nullptr) {
    printf("WARNING: No Module.symvers file found.\n");
  }

  if (Mode != LIST_ALL) {
    if (Symbols_To_Analyze.size() == 0) {
      printf("ERROR: No symbol to analyze.\n\n");
      Print_Usage();
    }
  }

  if (Output == DOT) {
    if (Mode == LIST_ALL) {
      printf("ERROR: Graphviz output requires -where-is-inlined or -compute-closure\n\n");
      Print_Usage();
    }
  }
}

static void Print_Symbol_Set(InlineAnalysis &ia, std::set<std::string> &set)
{
  if (Output == TERMINAL) {
    if (Output_Path == nullptr) {
      ia.Print_Symbol_Set(set);
    } else {
      FILE *out = fopen(Output_Path, "w");
      if (out == nullptr) {
        printf("ERROR: Unable to open output file %s\n", Output_Path);
        exit(1);
      }
      ia.Print_Symbol_Set(set, false, out);
      printf("Output written to %s\n", Output_Path);
      fclose(out);
    }
    return;
  }

  if (Output == CSV) {
    FILE *out = fopen(Output_Path, "w");
    if (out == nullptr) {
      printf("ERROR: Unable to open output file %s\n", Output_Path);
      exit(1);
    }
    ia.Print_Symbol_Set(set, true, out);
    printf("Output written to %s\n", Output_Path);
    fclose(out);

    return;
  }

  if (Output == DOT) {
    /* Should not be here.  */
    abort();
  }
}

int main(int argc, char *argv[])
{
  Parse(argc, argv);
  Check_Input();
  InlineAnalysis ia(Elf_Path, Ipa_Path, Symvers_Path);

  if (Mode == LIST_ALL) {
    std::set<std::string> set = ia.Get_All_Symbols();
    Print_Symbol_Set(ia, set);
    return 0;
  }

  if (Mode == WHERE_IS_INLINED) {
    if (Output == DOT) {
      ia.Get_Graphviz_Of_Inlines(Symbols_To_Analyze, Output_Path);
      printf("Output written to %s\n", Output_Path);
    } else {
      auto inline_set = ia.Get_Where_Symbols_Is_Inlined(Symbols_To_Analyze);
      Print_Symbol_Set(ia, inline_set);
    }

    return 0;
  }
  if (Mode == INLINE_CLOSURE) {
    if (Output == DOT) {
      ia.Get_Graphviz_Of_Inline_Closure(Symbols_To_Analyze, Output_Path);
      printf("Output written to %s\n", Output_Path);
    } else {
      auto inline_set = ia.Get_Inline_Closure_Of_Symbols(Symbols_To_Analyze);
      Print_Symbol_Set(ia, inline_set);
    }
    return 0;
  }

  __builtin_unreachable();
}
