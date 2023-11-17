/** InlineAnalysis: Do an analysis to check if symbols are inlined.
  *
  * This class works by merging content from the GCC ipa-clones dumps and the
  * debuginfo of the program, the first content being required and the second
  * being optional, only to improve the analysis quality.
  */

#pragma once

#include "ElfCXX.hh"
#include "IpaClonesParser.hh"
#include "SymversParser.hh"

#include <set>
#include <string>
#include <vector>
#include <stdio.h>

struct IpaCloneNode;

enum ExternalizationType {
  NONE = 0,
  WEAK,
  STRONG,
};

class InlineAnalysis
{
  public:
  /** Build the analysis class.  elf_path can be NULL if there is no debuginfo
      available, and ipaclone_path can be a directory full of many ipa-clones
      generated through LTO or not. Symvers can be NULL is we are creating a
      userspace livepatch   */
  InlineAnalysis(const char *elf_path, const char *ipaclone_path, const char *symvers_path);

  ~InlineAnalysis(void);

  /** Get a set of all functions that are inlined into `asm_name`.  */
  std::set<std::string> Get_Inline_Closure_Of_Symbol(const std::string &asm_name);

  /** Get a set of all functions that are inlined into the set of functions `syms`.  */
  std::set<std::string> Get_Inline_Closure_Of_Symbols(const std::vector<std::string> &syms);

  /** Get a set of all functions where a given function was inlined.  */
  std::set<std::string> Get_Where_Symbol_Is_Inlined(const std::string &asm_name);

  /** Get a set of all functions where a given set of functions was inlined.  */
  std::set<std::string> Get_Where_Symbols_Is_Inlined(const std::vector<std::string> &syms);

  /** Build a graphviz .dot file showing where the `functions` got inlined.  */
  void Get_Graphviz_Of_Inlines(const std::vector<std::string> &functions,
                               const char *output_path);

  /** Build a graphviz .dot file showing which functions got inlined into
      `functions`.  */
  void Get_Graphviz_Of_Inline_Closure(const std::vector<std::string> &functions,
                                      const char *output_path);

  /** Get the ELF info of a symbol.  */
  unsigned char Get_Symbol_Info(const std::string &sym);

  ExternalizationType Needs_Externalization(const std::string &sym);

  /** Check if symbol is externally visible.  */
  bool Is_Externally_Visible(const std::string &sym);

  /* Print the symbol set in a table-like format, for terminal output.  */
  void Print_Symbol_Set(const std::set<std::string> &set, bool csv=false, FILE *out=stdout);

  /* Get all symbols available to this class.  Look into debuginfo and
     ipa-clones.  */
  std::set<std::string> Get_All_Symbols(void);

  /** True if this class was built with debuginfo enabled.  */
  inline bool Have_Debuginfo(void)
  {
    return ElfObj && ElfCache;
  }

  /** Check if we have Ipa-clones information.  */
  inline bool Have_IPA(void)
  {
    return Ipa;
  }

  inline bool Have_Symvers(void)
  {
    return Symv;
  }

  inline bool Can_Decide_Visibility(void)
  {
    return Have_Debuginfo() || Have_Symvers();
  }

  /** Dump for debugging concerns.  */
  void Dump(void);


  /** Demangle C++ symbol.

      C++ symbols are mangled in order to be unique for the linker. For example,
      given:
        _ZNK7Point3D5Get_ZEv
      turns into:
        Point3D::Get_Z() const

      NOTE: returned value must be free.
  */
  static const char *Demangle_Symbol(const char *symbol);
  static inline const char *Demangle_Symbol(const std::string &symbol)
  {
    return Demangle_Symbol(symbol.c_str());
  }

  private:
  /** Put color information in the graphviz .DOT file.  */
  void Print_Node_Colors(const std::set<IpaCloneNode *> &set, FILE *fp);

  ElfObject *ElfObj;
  ElfSymbolCache *ElfCache;
  IpaClones *Ipa;
  Symvers *Symv;
};
