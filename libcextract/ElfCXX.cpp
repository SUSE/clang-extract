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

#include <zlib.h>
#include <zstd.h>

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
    DecompressedObj(nullptr),
    ElfFd(-1)
{
  /* Libelf quirks.  If not present it fails to load current Linux binaries.  */
  elf_version(EV_CURRENT);

  /* Open using Unix File Descriptor, as required by libelf.  */
  ElfFd = open(parser_path.c_str(), O_RDONLY);
  if (ElfFd == -1) {
    throw std::runtime_error("ELF file not found: " + parser_path);
  }

  enum FileHandling::FileType ft = FileHandling::Get_File_Type(ElfFd);

  switch (ft) {
  case FileHandling::FILE_TYPE_ELF: {
    /* Create libelf object and store it in the class variable.  */
    ElfObj = elf_begin(ElfFd, ELF_C_READ, nullptr);
    if (ElfObj == nullptr) {
      close(ElfFd);
      throw std::runtime_error("libelf error on file " + parser_path + ": "
                                   + std::string(elf_errmsg(elf_errno())));
    }
    break;
  }

  /* gzip magic number (zlib) */
  case FileHandling::FILE_TYPE_GZ: {
    try {
      ElfObj = decompress_gz();
      close(ElfFd);
      ElfFd = -1;
    } catch (const std::runtime_error &error) {
      ElfObject::~ElfObject();
      throw error;
    }
    break;
  }

  /* zstd magic number */
  case FileHandling::FILE_TYPE_ZSTD: {
    try {
      ElfObj = decompress_zstd();
      close(ElfFd);
      ElfFd = -1;
    } catch (const std::runtime_error &error) {
      ElfObject::~ElfObject();
      throw error;
    }
    break;
  }

  default:
    close(ElfFd);
    throw std::runtime_error("Format not recognized: " + parser_path + "\n");
  }
}

/* Destroy inner libelf structures.  */
ElfObject::~ElfObject(void)
{
  if (ElfObj) {
    /* ELF object is valid.  Destroy it.  */
    elf_end(ElfObj);
  }

  if (DecompressedObj)
    free(DecompressedObj);

  if (ElfFd != -1) {
    /* File Descriptor is still open.  */
    close(ElfFd);
    ElfFd = -1;
  }
}

Elf *ElfObject::decompress_gz(void)
{
  const size_t CHUNK = 16384;

  unsigned have;
  unsigned char in[CHUNK];
  unsigned char out[CHUNK];

  unsigned long dest_size = CHUNK;
  DecompressedObj = (unsigned char *)malloc(CHUNK);
  if (!DecompressedObj)
    throw std::runtime_error("zlib malloc failed\n");

  unsigned long dest_current = 0;

  z_stream strm;
  memset(&strm, 0, sizeof(strm));

  /* Allocate inflate state. The size is related to how zlib inflates gzip files. */
  int ret = inflateInit2(&strm, 16+MAX_WBITS);
  if (ret != Z_OK)
    throw std::runtime_error("zlib inflateInit failed\n");

  /* decompress until deflate stream ends or end of file */
  do {
    strm.avail_in = read(ElfFd, in, CHUNK);
    if (strm.avail_in < 0)
      throw std::runtime_error("zlib read failed: " + std::to_string(strm.avail_in) + "\n");

    if (strm.avail_in == 0)
      break;
    strm.next_in = in;

    /* run inflate() on input until output buffer not full */
    do {
      strm.avail_out = CHUNK;
      strm.next_out = out;
      ret = inflate(&strm, Z_NO_FLUSH);

      switch (ret) {
        case Z_NEED_DICT:
          ret = Z_DATA_ERROR;     /* and fall through */
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
          inflateEnd(&strm);
          throw std::runtime_error("zlib inflate error: " + std::to_string(ret) + "\n");
      }

      have = CHUNK - strm.avail_out;

      /* double the buffer when needed */
      if (have > dest_size - dest_current) {
        dest_size = dest_size * 2;
        DecompressedObj = (unsigned char *)realloc(DecompressedObj, dest_size);
      }

      memcpy(DecompressedObj + dest_current, out, have);
      dest_current += have;
    } while (strm.avail_out == 0);

    /* done when inflate() says it's done */
  } while (ret != Z_STREAM_END);

  inflateEnd(&strm);

  if (ret != Z_STREAM_END)
      throw std::runtime_error("zlib inflateEnd error: " + std::to_string(ret) + "\n");

  Elf *elf = elf_memory((char *)DecompressedObj, ret);
  if (elf == nullptr)
    throw std::runtime_error("libelf elf_memory error: " + std::string(elf_errmsg(elf_errno())));

  return elf;
}

Elf *ElfObject::decompress_zstd()
{
  size_t buffInSize = ZSTD_DStreamInSize();
  unsigned char buffIn[buffInSize];

  size_t buffOutSize = ZSTD_DStreamOutSize();
  unsigned char buffOut[buffOutSize];

  unsigned long dest_size = buffOutSize;
  DecompressedObj = (unsigned char *)malloc(dest_size);
  if (!DecompressedObj)
    throw std::runtime_error("zstd malloc failed\n");

  unsigned long dest_current = 0;

  ZSTD_DCtx* dctx = ZSTD_createDCtx();
  if (!dctx)
    throw std::runtime_error("zstd createDCtx failed\n");

  size_t bytes_read;
  while ((bytes_read = read(ElfFd, buffIn, buffInSize)) ) {

    if (bytes_read < 0) {
      ZSTD_freeDCtx(dctx);
      throw std::runtime_error("ZSTD read failed: " + std::to_string(bytes_read) + "\n");
    }

    ZSTD_inBuffer input = { buffIn, bytes_read, 0 };

    while (input.pos < input.size) {
      ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };

      size_t ret = ZSTD_decompressStream(dctx, &output , &input);
      if (ZSTD_isError(ret)) {
        ZSTD_freeDCtx(dctx);
        throw std::runtime_error("ZSTD_decompressStream failed: " + std::string(ZSTD_getErrorName(ret)) + "\n");
      }

      if (output.size > dest_size - dest_current) {
        dest_size = dest_size * 2;
        DecompressedObj = (unsigned char *)realloc(DecompressedObj, dest_size);
      }

      memcpy(DecompressedObj + dest_current, buffOut, output.pos);
      dest_current += output.size;
    }
  }

  ZSTD_freeDCtx(dctx);

  Elf *elf = elf_memory((char *)DecompressedObj, dest_size);
  if (elf == nullptr)
    throw std::runtime_error("libelf elf_memory error: " + std::string(elf_errmsg(elf_errno())));

  return elf;
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

/** Get symbol if available in either symtab.  Returns in which symtab this
    symbol was found.  */
std::pair<unsigned char, ElfSymtabType>
ElfSymbolCache::Get_Symbol_Info(const std::string &sym)
{
  unsigned char ret = 0;

  /* Try the dynsym first, which means this symbol is most likely public
     visible.  */
  ret = Get_Symbol_Info_Dynsym(sym);
  if (ret != 0) {
    return std::make_pair(ret, ElfSymtabType::DYNSYM);
  }

  /* Symbol not available on dynsym.  Try symtab.  */
  ret = Get_Symbol_Info_Symtab(sym);
  if (ret != 0) {
    return std::make_pair(ret, ElfSymtabType::SYMTAB);
  }

  /* If symbol is not here there is nothing I can do.  */
  return std::make_pair(0, ElfSymtabType::TAB_NONE);
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
