//===- ElfCXX.cpp - C++ Wrapper for libelf ----------------------*- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// A C++ wrapper for libelf.
//
//===----------------------------------------------------------------------===//

/* Authors: Giuliano Belinassi, Marcos Paulo de Souza  */

#include "ElfCXX.hh"
#include "NonLLVMMisc.hh"
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdexcept>
#include <iostream>
#include <string.h>

const char *ElfSymbol::Get_Name(void)
{
  struct Elf *elf = ElfObj.Get_Wrapped_Object();
  return elf_strptr(elf, Shlink, Symbol.st_name);
}

const char *ElfSymbol::Type_As_String(unsigned info)
{
  /** Table with all possible valuies of info.  */
  static const char *const tbl[16] = {
    "NOTYPE",   // No known type.
    "OBJECT",   // Is an object.
    "FUNC",     // Is a function.
    "SECTION",  // Is a section.
    "FILE",     // Is a file.
    "COMMON",   // ??
    "TLS",      // Thread Locas Storage variable.
    "NUM",      // Constant number;
    nullptr,
    nullptr,
    "IFUNC",    // Store a reference to a function
    nullptr,
    "HIOS",     // ??
    "LOPROC",
    nullptr,    // ??
    "HIPROC",
  };

  return tbl[info];
}

const char *ElfSymbol::Bind_As_String(unsigned link)
{
  /** Table with all possible valuies of bind.  */
  static const char *const tbl[14] = {
    "LOCL",   // Available in the ELF, but not linkable.
    "GLOB",   // Available in ELF and is linkable.
    "WEAK",   // Not available in ELF, will be provided by a library.
    "NUM",    // ??
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "LOOS",   // ??
    "HIOS",   // ??
    "LOPROC", // ??
    "HIPROC", // ??
  };

  return tbl[link];
}

/* Build the ELF object from the given path.  */
ElfObject::ElfObject(const char *path)
  : Parser(path),
    ElfObj(nullptr),
    ElfFd(-1)
{
  /* Libelf quirks.  If not present it fails to load current Linux binaries.  */
  elf_version(EV_CURRENT);

  /* Open using Unix File Descriptor, as required by libelf.  */
  ElfFd = open(parser_path.c_str(), O_RDONLY);
  if (ElfFd == -1) {
    throw std::runtime_error("ELF file not found: " + parser_path);
  }

  /* Create libelf object and store it in the class variable.  */
  ElfObj = elf_begin(ElfFd, ELF_C_READ, nullptr);
  if (ElfObj == nullptr) {
    close(ElfFd);
    throw std::runtime_error("libelf error on file " + parser_path + ": "
                                 + std::string(elf_errmsg(elf_errno())));
  }
}

/* Destroy inner libelf structures.  */
ElfObject::~ElfObject(void)
{
  if (ElfObj) {
    /* ELF object is valid.  Destroy it.  */
    elf_end(ElfObj);
  }

  if (ElfFd != -1) {
    /* File Descriptor is still open.  */
    close(ElfFd);
    ElfFd = -1;
  }
}

/** Get the next ELF section.  ELF is a multisection file and we need to
    iterate if we want to know certain information.  */
ElfSection ElfSection::Get_Next_Section(void)
{
  return ElfSection(ElfObj, elf_nextscn(ElfObj.Get_Wrapped_Object(), Section));
}

/** Go to the next ELF section without rebuilding this object.  */
void ElfSection::To_Next_Section(void)
{
  Section = elf_nextscn(ElfObj.Get_Wrapped_Object(), Section);
  gelf_getshdr(Section, &SectionHeader);
}

/** Get the name of the section.  */
const char *ElfSection::Get_Name(void)
{
  /* TODO: This can be optimized.  */
  size_t shstrndx;
  struct Elf *elf = ElfObj.Get_Wrapped_Object();
  if (elf_getshdrstrndx(elf, &shstrndx)) {
    return nullptr;
  }

  return elf_strptr(elf, shstrndx, SectionHeader.sh_name);
}

/* Cache the symbols from section.  */
void ElfSymbolCache::Insert_Symbols_Into_Hash(SymbolTableHash &map, ElfSection &section)
{
  size_t n = section.Get_Num_Symbols();
  for (size_t i = 0; i < n; i++) {
    ElfSymbol symbol = section.Get_Symbol(i);
    if (symbol.Get_Type() == STT_FILE) {
      /* Skip the file symbols, we don't need them.  */
      continue;
    }

    std::string symbol_name(symbol.Get_Name());
    map[symbol_name] = symbol.Get_Info();
  }
}

ElfSymbolCache::ElfSymbolCache(ElfObject &eo)
  : EO(eo)
{
  /* Look for dynsym and symtab sections.  */
  for (auto it = EO.section_begin(); it != EO.section_end(); ++it)
  {
    ElfSection &section = *it;
    switch (section.Get_Section_Type()) {
      case SHT_DYNSYM:
        Insert_Symbols_Into_Hash(DynsymMap, section);
        break;

      case SHT_SYMTAB:
        Insert_Symbols_Into_Hash(SymtabMap, section);
        break;

      case SHT_PROGBITS:
        /*
         * Search for the module name, if the ELF object is a kernel module. The
         * module name starts at offset 24 for 64bit kernels.
         */
        if (!strncmp(section.Get_Name(), ".gnu.linkonce.this_module", 25))
          Mod = std::string((char *)section.Get_Data()->d_buf + 24);

        break;
      default:
        break;
    }
  }
}

std::vector<std::string> ElfSymbolCache::Get_All_Symbols(void)
{
  std::vector<std::string> vec;

  for (auto const &i : DynsymMap) {
    vec.push_back(i.first);
  }

  for (auto const &i : SymtabMap) {
    vec.push_back(i.first);
  }

  return vec;
}

void Print_Section(ElfSection &section)
{
  size_t n = section.Get_Num_Symbols();

  for (unsigned i = 0; i < n; i++) {
    ElfSymbol symbol = section.Get_Symbol(i);
    std::cout << symbol.Get_Name() << '\n';
  }
}

void ElfSymbolCache::Dump_Cache(void)
{
  std::cout << "DynsymMap:\n";
  for (auto const &i : DynsymMap) {
    std::cout << i.first << "    " << ElfSymbol::Type_As_String(ELF64_ST_TYPE(i.second))
                         << "    " << ElfSymbol::Bind_As_String(ELF64_ST_BIND(i.second))
                         << '\n';
  }

  std::cout << "SymtabMap:\n";
  for (auto const &i : SymtabMap) {
    std::cout << i.first << "    " << ElfSymbol::Type_As_String(ELF64_ST_TYPE(i.second))
                         << "    " << ElfSymbol::Bind_As_String(ELF64_ST_BIND(i.second))
                         << '\n';
  }
}
