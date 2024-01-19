//===- InlineAnalysis.cpp - Find out which functions got inlined -*- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Use the information from debuginfo, ipa-clones and symvers to decide if
/// symbol is inlined, private or public in the target livepatch binary.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#include "InlineAnalysis.hh"
#include "NonLLVMMisc.hh"
#include <iostream>
#include <cxxabi.h>
#include <stdlib.h>
#include <cxxabi.h>
#include <stdexcept>

InlineAnalysis::InlineAnalysis(const char *elf_path, const char *ipaclones_path,
                              const char *symvers_path, bool is_kernel)
  : ElfObj(nullptr),
    ElfCache(nullptr),
    Ipa(nullptr),
    Symv(nullptr),
    Kernel(is_kernel)
{
  try {
    /* Debuginfo information is not needed for inline analysis.  But is desired
       for better precision.  That is why whe declare those objects dynamically.  */
    if (elf_path) {
      ElfObj = new ElfObject(elf_path);
      ElfCache = new ElfSymbolCache(*ElfObj);
    }

    if (ipaclones_path) {
      Ipa = new IpaClones(ipaclones_path);
    }

    if (symvers_path) {
      Symv = new Symvers(symvers_path);
    }
  } catch (std::runtime_error &e) {
    /* So what happens if this constructor throws an exception (like file not
       found) is that the destructor is never called and therefore it generates
       memory leaks.  But we *can* call the destructor here because it was
       designed to be safely callable if the object was not properly constructed
       and hence we call it here.  */
    this->~InlineAnalysis();

    /* Throw the error up for someone else to catch.  */
    throw e;
  }
}

InlineAnalysis::~InlineAnalysis(void)
{
  if (ElfCache)
    delete ElfCache;
  if (ElfObj)
    delete ElfObj;
  if (Ipa)
    delete Ipa;
  if (Symv)
    delete Symv;
}

static int Action_Add_Node2(void *s, IpaCloneNode *n1, IpaCloneNode *n2)
{
  (void) n1;

  std::set<std::string> *set = static_cast<std::set<std::string> *>(s);
  set->insert(n2->Name);

  return 0;
}

std::set<std::string> InlineAnalysis::Get_Inline_Closure_Of_Symbol(const std::string &asm_name)
{
  std::set<std::string> set;

  /* If we don't have the IPA information there is nothing we can do.  */
  if (Ipa == nullptr) {
    return set;
  }

  IpaClosure(*Ipa, (void*) &set, Action_Add_Node2).Find_Inlined_Symbols(asm_name);
  return set;
}

std::set<std::string> InlineAnalysis::Get_Inline_Closure_Of_Symbols(const std::vector<std::string> &symbols)
{
  std::set<std::string> set;
  /* If we don't have the IPA information there is nothing we can do.  */
  if (Ipa == nullptr) {
    return set;
  }

  IpaClosure closure(*Ipa, (void*) &set, Action_Add_Node2);
  for (const std::string &sym : symbols) {
    closure.Find_Inlined_Symbols(sym);
  }

  return set;
}

std::set<std::string> InlineAnalysis::Get_Where_Symbol_Is_Inlined(const std::string &asm_name)
{
  std::set<std::string> set;
  /* If we don't have the IPA information there is nothing we can do.  */
  if (Ipa == nullptr) {
    return set;
  }

  IpaClosure(*Ipa, (void *) &set, Action_Add_Node2)
    .Find_Where_Symbol_Is_Inlined(asm_name);

  return set;
}

std::set<std::string> InlineAnalysis::Get_Where_Symbols_Is_Inlined(const std::vector<std::string> &symbols)
{
  std::set<std::string> set;

  /* If we don't have the IPA information there is nothing we can do.  */
  if (Ipa == nullptr) {
    return set;
  }

  IpaClosure closure(*Ipa, (void *) &set, Action_Add_Node2);
  for (const std::string &sym : symbols) {
    closure.Find_Where_Symbol_Is_Inlined(sym);
  }

  return set;
}

static int Action_Graphviz(void *s, IpaCloneNode *n1, IpaCloneNode *n2)
{
  FILE *file = static_cast<FILE*>(s);

  const char *name1 = InlineAnalysis::Demangle_Symbol(n1->Name.c_str());
  const char *name2 = InlineAnalysis::Demangle_Symbol(n2->Name.c_str());
  fprintf(file, "\n\"%s\" -> \"%s\"", name1, name2);

  free(name1);
  free(name2);
  return 0;
}


void InlineAnalysis::Print_Node_Colors(const std::set<IpaCloneNode *> &set, FILE *fp)
{
  if (!Have_Debuginfo()) {
    return;
  }

  for (IpaCloneNode *node : set) {
    const char *demangled = InlineAnalysis::Demangle_Symbol(node->Name.c_str());
    unsigned char syminfo = Get_Symbol_Info(node->Name);
    if (syminfo == 0) {
      fprintf(fp, "\n\"%s\" [style=dotted]", demangled);
    } else {
      //unsigned char type = ElfSymbol::Type_Of(syminfo);
      unsigned char bind = ElfSymbol::Bind_Of(syminfo);
      if (bind == STB_LOCAL) {
        fprintf(fp, "\n\"%s\" [color=red]", demangled);
      } else if (bind == STB_GLOBAL) {
        fprintf(fp, "\n\"%s\" [color=black]", demangled);
      } else if (bind == STB_WEAK) {
        fprintf(fp, "\n\"%s\" [color=green]", demangled);
      } else {
        fprintf(fp, "\n\"%s\" [color=yellow]", demangled);
      }
    }
    free(demangled);
  }
}

void InlineAnalysis::Get_Graphviz_Of_Inlines(const std::vector<std::string> &symbols, const char *output_path)
{
  FILE *file = fopen(output_path, "w");
  if (file == nullptr) {
    throw std::runtime_error("ERROR: Unable to open file " +
                             std::string(output_path) +
                             " to write");
  }

  fprintf(file, "strict digraph {");
  IpaClosure closure(*Ipa, (void *) file, Action_Graphviz);
  for (const std::string &sym : symbols) {
    closure.Find_Where_Symbol_Is_Inlined(sym);
  }

  Print_Node_Colors(closure.Set, file);

  fprintf(file, "\n}");
  fclose(file);
}

static int Action_Graphviz_Reverse(void *s, IpaCloneNode *n1, IpaCloneNode *n2)
{
  FILE *file = static_cast<FILE*>(s);
  const char *name1 = InlineAnalysis::Demangle_Symbol(n2->Name.c_str());
  const char *name2 = InlineAnalysis::Demangle_Symbol(n1->Name.c_str());

  fprintf(file, "\n\"%s\" -> \"%s\"", name1, name2);

  free(name1);
  free(name2);
  return 0;
}

void InlineAnalysis::Get_Graphviz_Of_Inline_Closure(const std::vector<std::string> &symbols, const char *output_path)
{
  FILE *file = fopen(output_path, "w");
  if (file == nullptr) {
    throw std::runtime_error("Unable to open file " +
                             std::string(output_path) +
                             " to write");
  }

  fprintf(file, "strict digraph {");
  IpaClosure closure(*Ipa, (void *) file, Action_Graphviz_Reverse);
  for (const std::string &sym : symbols) {
    closure.Find_Inlined_Symbols(sym);
  }

  Print_Node_Colors(closure.Set, file);

  fprintf(file, "\n}");
  fclose(file);
}


void InlineAnalysis::Dump(void)
{
  if (Ipa) {
    Ipa->Dump();
  } else {
    std::cout << "IPA is NULL\n";
  }
}

unsigned char InlineAnalysis::Get_Symbol_Info(const std::string &sym)
{
  /* Try the dynsym first, which means this symbol is most likely publically
     visible.  */
  unsigned char ret = 0;
  if (ElfCache == nullptr) {
    /* If we don't have the ElfCache there is nothing we can do.  */
    return ret;
  }

  ret = ElfCache->Get_Symbol_Info_Dynsym(sym);
  if (ret != 0) {
    return ret;
  }

  /* Symbol not available on dynsym.  Try symtab.  */
  ret = ElfCache->Get_Symbol_Info_Symtab(sym);

  /* If symbol is not here there is nothing I can do.  */
  return ret;
}

static const char *Bind(unsigned link)
{
  switch (link) {
    case STB_LOCAL:
      return "Private symbol";
    break;

    case STB_GLOBAL:
      return "Public symbol";
    break;

    case STB_WEAK:
      return "Extern symbol";
    break;

    default:
      /* If the symbol is not available on the debuginfo then we consider it as
         inlined.  */
      return "Inlined";
    break;
  }
}

ExternalizationType InlineAnalysis::Needs_Externalization(const std::string &sym)
{
  if (Symv) {
    const std::string &sym_mod = Symv->Get_Symbol_Module(sym);
    /*
     * If the symbol exists on Symvers we can decide whether the symbol must be
     * externalized or not, and not rely on ELF.
     */
    if (!sym_mod.empty())
      return (Symv->Needs_Externalization(sym_mod)) ? ExternalizationType::STRONG
                                                    : ExternalizationType::NONE;
  }

  unsigned char info = Get_Symbol_Info(sym);
  if (info > 0) {
    unsigned bind = ElfSymbol::Bind_Of(info);
    switch (bind) {
      case STB_GLOBAL:
        /*
         * When creating a livepatch for Linux kernel, we need to externalize
         * all existing symbols that don't reside on vmlinux. In this case, the
         * specific symbol was found in the ELF object, so we can safely externalize
         * it.
         */
        if (Kernel) {
          return ExternalizationType::STRONG;
        }
        return ExternalizationType::WEAK;
        break;
      case STB_LOCAL:
        return ExternalizationType::STRONG;
        break;

      case STB_WEAK:
      default:
        return ExternalizationType::NONE;
        break;
    }
  }

  /* No debuginfo provided, there is nothing we can do.  */
  return ExternalizationType::NONE;
}

bool InlineAnalysis::Is_Externally_Visible(const std::string &sym)
{
  return Needs_Externalization(sym) != ExternalizationType::STRONG;
}


std::set<std::string> InlineAnalysis::Get_All_Symbols(void)
{
  /* We have to use the information that the user provide us.  This means that
     if the debuginfo or the ipaclones are not available, we have to do the
     best we can.  */

  std::set<std::string> set;

  if (Have_Debuginfo()) {
    for (auto sym : ElfCache->Get_All_Symbols())
	    set.insert(sym);
  }

  if (Ipa) {
    for (auto node_map : *Ipa) {
      const std::string &sym = node_map.first;
      set.insert(sym);
    }
  }

  if (Symv) {
    for (auto &sym : Symv->Get_All_Symbols())
	    set.insert(sym);
  }

  return set;
}

void InlineAnalysis::Print_Symbol_Set(const std::set<std::string> &symbol_set,
                                      bool csv, FILE *out)
{
  bool have_debuginfo = Have_Debuginfo();
  size_t max_symbol_chars = 15;
  size_t max_symbol_chars_mangled = 15;
  size_t num_symbols = symbol_set.size();

  if (num_symbols == 0) {
    /* No symbols to print.  */
    return;
  }

  const char *demangleds[num_symbols];
  size_t i = 0;

  for (std::string s : symbol_set) {
    const char *demangled = InlineAnalysis::Demangle_Symbol(s.c_str());
    max_symbol_chars = std::max(max_symbol_chars, strlen(demangled));
    max_symbol_chars_mangled = std::max(max_symbol_chars_mangled, s.length());

    demangleds[i++] = demangled;
  }

// Ignore warnings related to printf formating.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
  /* Print table header.  */
  fprintf(out,"Mangled name");
  if (csv) {
    fprintf(out,";");
  } else {
    size_t j = max_symbol_chars_mangled + 1 - strlen("Mangled name");
    fprintf(out, std::string(j, ' ').c_str());
  }

  fprintf(out,"Demangled name");
  if (csv) {
    fprintf(out,";");
  } else {
    size_t s = max_symbol_chars + 1 - strlen("Demangled name");
    fprintf(out, std::string(s, ' ').c_str());
  }

  if (have_debuginfo) {
    if (csv) {
      fprintf(out,"Type;Available?");
    } else {
      fprintf(out,"Type\tAvailable?");
    }
  } else if (Symv) {
    fprintf(out, "Module");
  }
  fprintf(out, "\n");

  i = 0;
  for (std::string s : symbol_set) {
    /* Print mangled name.  */
    fprintf(out,"%s", s.c_str());
    if (csv) {
      fprintf(out,";");
    } else {
      size_t j = max_symbol_chars_mangled + 1 - s.length();
      fprintf(out, std::string(j, ' ').c_str());
    }

    /* Print demangled name.  */
    const char *demangled = demangleds[i++];
    size_t len = strlen(demangled);
    fprintf(out,"%s", demangled);
    if (csv) {
      fprintf(out,";");
    } else {
      size_t j = max_symbol_chars + 1 - len;
      fprintf(out, std::string(j, ' ').c_str());
    }

    /* In case we have debuginfo, also print if symbols was completely inlined.  */
    if (have_debuginfo) {
      unsigned char syminfo = Get_Symbol_Info(s);
      unsigned type = ElfSymbol::Type_Of(syminfo);
      unsigned bind = ElfSymbol::Bind_Of(syminfo);

      const char *type_str = ElfSymbol::Type_As_String(type);
      const char *bind_str = (syminfo > 0) ? Bind(bind) : "Inlined";

      if (csv) {
        fprintf(out,"%s;%s\n", type_str, bind_str);
      } else {
        fprintf(out,"%s\t%s\n", type_str, bind_str);
      }
    } else if (Symv) {
      fprintf(out, "%s\n", Symv->Get_Symbol_Module(s).c_str());
    } else {
      fprintf(out,"\n");
    }

    free(demangled);
  }
#pragma GCC diagnostic pop
}


const char *InlineAnalysis::Demangle_Symbol(const char *symbol)
{
  int status;
  const char *demangled = abi::__cxa_demangle(symbol, nullptr, nullptr, &status);
  if (status == -2) {
    return strdup(symbol);
  } else {
    return demangled;
  }
}

/*
 * Check if Kernel module was found on Symvers or ELF object. Returns empty is
 * the module was not found, or if the LP is not from a kernel source.
 */
std::string InlineAnalysis::Get_Symbol_Module(std::string sym)
{
  std::string mod;
  if (Symv) {
    mod = Symv->Get_Symbol_Module(sym);
    if (!mod.empty())
      return mod;
  }

  if (Have_Debuginfo())
    return ElfCache->Get_Symbol_Module(sym);

  return {};
}
