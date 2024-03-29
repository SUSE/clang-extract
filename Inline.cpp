//===- Inline.cpp - Show inlining decisions and symbol visibility --*- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Show inlining decisions and symbol visibility.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#include "InlineAnalysis.hh"
#include "NonLLVMMisc.hh"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

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
"     -graphviz                Output as a .dot graphviz format,\n"
"     -csv                     Output as a .csv table format,\n"
"     -where-is-inlined        Find where <SYMBOLS> got inlined,\n"
"     -compute-closure         Find symbols that got inlined into <SYMBOLS>,\n"
"     -o         <PATH>        Output to file in <PATH>.\n"
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

#define NULL_OR_EMPTY_STRING()

static int Check_Input(void)
{
  if (is_null_or_empty(Ipa_Path)) {
    printf("WARNING: No IPA files found.\n");
  }

  if (is_null_or_empty(Elf_Path)) {
    printf("WARNING: No debuginfo file found.\n");
  }

  if (is_null_or_empty(Symvers_Path)) {
    printf("WARNING: No Module.symvers file found.\n");
  }

  if (Mode != LIST_ALL) {
    if (Symbols_To_Analyze.size() == 0) {
      printf("ERROR: No symbol to analyze.\n\n");
      Print_Usage();
      return 1;
    }
  }

  if (Output == DOT) {
    if (Mode == LIST_ALL) {
      printf("ERROR: Graphviz output requires -where-is-inlined or -compute-closure\n\n");
      Print_Usage();
      return 1;
    }
  }

  if (is_null_or_empty(Elf_Path) &&
      is_null_or_empty(Ipa_Path) &&
      is_null_or_empty(Symvers_Path)) {
      printf("ERROR: Please inform -debuginfo, -ipa-files or -symvers option.\n\n");
      Print_Usage();
      return 1;
  }

  return 0;
}

static void Print_Symbol_Set(InlineAnalysis &ia, std::set<std::string> &set)
{
  if (Output == TERMINAL) {
    if (Output_Path== nullptr) {
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
  if (int ret = Check_Input()) {
    return ret;
  }

  try {

    InlineAnalysis ia(Elf_Path, Ipa_Path, Symvers_Path, false);

    if (Mode == LIST_ALL) {
      std::set<std::string> set = ia.Get_All_Symbols();
      Print_Symbol_Set(ia, set);
      return 0;
    }

    if (Mode == WHERE_IS_INLINED) {
      if (Output == DOT) {
        if (Output_Path == nullptr) {
          printf("ERROR: No output path\n");
          exit(1);
        }

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
        if (Output_Path == nullptr) {
          printf("ERROR: No output path\n");
          exit(1);
        }
        ia.Get_Graphviz_Of_Inline_Closure(Symbols_To_Analyze, Output_Path);
        printf("Output written to %s\n", Output_Path);
      } else {
        auto inline_set = ia.Get_Inline_Closure_Of_Symbols(Symbols_To_Analyze);
        Print_Symbol_Set(ia, inline_set);
      }
      return 0;
    }
  } catch (std::runtime_error &err) {
    printf("ERROR: %s\n", err.what());
    abort();
  }

  __builtin_unreachable();
}
