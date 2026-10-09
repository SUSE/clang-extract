//===- ElfCXX.hh- C++ Wrapper for libelf -----------------------*- C++ -*-===//
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


/** ElfCXX: Wrapper class for libelf.
 *
 * Libelf is a library for handling ELF files written in C. It provides a
 * basic API for handling ELF files which IMHO is not straightfoward.
 * Those classes tries to make things easier by collapsing many structures
 * together (e.g. Section, Section Header and Section Data into a single class).
 *
 * Author: Giuliano Belinassi, Marcos Paulo de Souza
 */

#pragma once

#include "Parser.hh"

#include <elf.h>
#include <libelf.h>
#include <gelf.h>
#include <link.h>
#include <string>
#include <iterator>
#include <unordered_map>

class ElfObject;
class ElfSection;
class ElfSymbol;
class ElfVerdef;


/** The symbol table in which this symbol was found.  */
enum ElfSymtabType {
  TAB_NONE = 0,
  SYMTAB = SHT_SYMTAB,
  DYNSYM = SHT_DYNSYM,
};

/** @brief ELF symbol.
  *
  * An ELF executable may contain multiple symbols (variables, functions, ...).
  * This class represent this.
  *
  * NOTE: run $ readelf -sW <binary> to show the symbols.
  */
class ElfSymbol
{
  public:
  inline ElfSymbol(ElfObject &eo, ElfW(Word) shlink, GElf_Sym &symbol)
    : ElfObj(eo),
      Shlink(shlink),
      Symbol(symbol)
  {
  }

  /** Get name of the symbol.  */
  const char *Get_Name(void);

  /** Get original Elf_Sym object from libelf.  */
  inline GElf_Sym *Get_Wrapped_Object(void)
  {
    return &Symbol;
  }

  /** Get ELF object from which this symbol is from.  */
  inline ElfObject &Get_Elf_Object(void)
  {
    return ElfObj;
  }

  /** Get symbol `info` (packs `type` and `bind`).  */
  [[gnu::pure]] inline unsigned char Get_Info(void)
  {
    return Symbol.st_info;
  }

  /** Get the symbol `type` (func, object, ...).  */
  inline unsigned char Get_Type(void)
  {
    return ELF64_ST_TYPE(Get_Info());
  }

  /** Get the symbol `bind` (global, weak, ...).  */
  inline unsigned char Get_Bind(void)
  {
    return ELF64_ST_BIND(Get_Info());
  }

  static unsigned char Type_Of(unsigned char info)
  {
    return ELF64_ST_TYPE(info);
  }

  static unsigned char Bind_Of(unsigned char info)
  {
    return ELF64_ST_BIND(info);
  }

  /** Get type as string.  */
  static const char *Type_As_String(unsigned info);
  /** Get bind as string.  */
  static const char *Bind_As_String(unsigned link);

  /** Get type as string.  */
  inline const char *Get_Type_As_String(void)
  {
    return Type_As_String(Get_Type());
  }

  /** Get bind as string.  */
  inline const char *Get_Bind_As_String(void)
  {
    return Bind_As_String(Get_Bind());
  }

  private:

  /** Reference to the ELF object (file).  */
  ElfObject &ElfObj;

  /** Shared Header link number.  Used to compute the symbol name.  */
  ElfW(Word) Shlink;

  /** Original libelf symbol object.  */
  GElf_Sym Symbol;
};

/** @brief ELF version definition.
  *
  * Wraps a GElf_Verdef entry of a .gnu.version_d section, which defines a
  * symbol version (e.g. GLIBC_2.34, GLIBC_PRIVATE).  Entries form a chained
  * list inside the section and can be walked with To_Next_Verdef.
  */
class ElfVerdef
{
  public:
  /** Build the object pointing to the first verdef entry of `section`.  */
  inline ElfVerdef(ElfSection &section);

  /** Check if there is another verdef entry after this one.  */
  inline bool Have_Next_Verdef(void)
  {
    return Verdef.vd_next != 0;
  }

  /** Advance to the next verdef entry.  Returns false if there is none.  */
  inline bool To_Next_Verdef(void);

  /** Get the version name (e.g. GLIBC_PRIVATE) of this entry.  */
  const char *Get_Version_Name(void);

  /** Get the version index, which is referenced by the .gnu.version entries.  */
  inline unsigned Get_Ndx(void)
  {
    return Verdef.vd_ndx;
  }

  inline friend bool operator==(const ElfVerdef &a, const ElfVerdef &b)
  {
    return memcmp(&a.Verdef, &b.Verdef, sizeof(GElf_Verdef)) == 0;
  }

  inline friend bool operator!=(const ElfVerdef &a, const ElfVerdef &b)
  {
    return memcmp(&a.Verdef, &b.Verdef, sizeof(GElf_Verdef)) != 0;
  }

  private:

  /** The .gnu.version_d section this entry belongs to.  */
  ElfSection &Section;

  /** Byte offset of this entry within the section data.  */
  size_t Offset;

  /** The verdef entry read from libelf.  */
  GElf_Verdef Verdef;
};

/** @brief ELF section.
  *
  * An ELF executable may contain multiple section (.plt, .symtab, .dynamic, ...)
  * This class represent this.
  *
  * NOTE: run $ readelf -S <binary> to show all sections in a binary.
  */
class ElfSection
{
  public:
  inline ElfSection(ElfObject &eo, Elf_Scn *section)
    : ElfObj(eo),
      Data(nullptr),
      Section(section)
  {
    /** Get Section Header from the Section.  */
    gelf_getshdr(Section, &SectionHeader);
  }

  /** Get Section Data.  */
  inline Elf_Data *Get_Data(void)
  {
    if (Data == nullptr)
      Data = elf_getdata(Section, nullptr);

    return Data;
  }

  /** Get the next section.  */
  ElfSection Get_Next_Section(void);

  /** Move this class to the next section object.  */
  void To_Next_Section(void);

  /** Check if this section is a symbol table (dynsym, symtab).  */
  inline bool Is_Symbol_Table(void)
  {
    ElfW(Word) type = SectionHeader.sh_type;
    return type == SHT_SYMTAB || type == SHT_DYNSYM;
  }

  inline ElfW(Word) Get_Section_Type(void)
  {
    return SectionHeader.sh_type;
  }

  /** Get the section header link (e.g. the string table index).  */
  inline ElfW(Word) Get_Link(void)
  {
    return SectionHeader.sh_link;
  }

  /** Get name of this section.  */
  const char *Get_Name(void);

  /** Get Elf Object for this Section.  */
  inline ElfObject &Get_Elf_Object(void)
  {
    return ElfObj;
  }

  /** Get the number of symbols in this section, if it is a symbol table.  */
  inline size_t Get_Num_Symbols(void)
  {
    return SectionHeader.sh_size / SectionHeader.sh_entsize;
  }

  /** Get the i-th symbol in this section.  */
  inline ElfSymbol Get_Symbol(unsigned i)
  {
    GElf_Sym sym;
    gelf_getsym(Get_Data(), i, &sym);
    return ElfSymbol(ElfObj, SectionHeader.sh_link, sym);
  }

  /** Get the original Elf_Scn object from libelf.  */
  inline Elf_Scn *Get_Wrapped_Object(void)
  {
    return Section;
  }

  /** Check if two sections are equal.  */
  inline friend bool operator==(const ElfSection &a, const ElfSection &b)
  {
    return a.Section == b.Section;
  }

  /** Check if two sections are not equal.  */
  inline friend bool operator!=(const ElfSection &a, const ElfSection &b)
  {
    return a.Section != b.Section;
  }

  /** Iterator over the version definitions (Elf_Verdef) of a SHT_GNU_verdef
      section.  */
  struct VerdefIterator
  {
    public:
    /** Iterator pointing to the first version definition of `section`.  */
    VerdefIterator(ElfSection &section)
      : AtEnd(false), Verdef(section)
    {
    }

    /** End-of-list sentinel iterator.  The pointer argument is only used to
        distinguish this constructor from the one above.  */
    VerdefIterator(ElfSection &section, void *)
      : AtEnd(true), Verdef(section)
    {
      /* Create invalid object.  */
      memset(&Verdef, 0, sizeof(Verdef));
    }

    /** Doing `*it` should access the Elf_Verdef object.  */
    ElfVerdef &operator*()
    {
      return Verdef;
    }

    /** Doing `*it` should access the Elf_Verdef object (const version).  */
    const ElfVerdef &operator*() const
    {
      return Verdef;
    }

    /** Prefix iterator (++it).  Becomes the end iterator once there are no
        more version definitions.  */
    VerdefIterator &operator++(void)
    {
      AtEnd = !Verdef.To_Next_Verdef();
      return *this;
    }

#if 0
    inline friend bool operator==(const VerdefIterator &a, const VerdefIterator &b)
    {
      return a.Verdef == b.Verdef;
    }
#endif

    /** Only the end state is compared, which is enough for comparing against
        verdef_end() in loops.  */
    inline friend bool operator!=(const VerdefIterator &a, const VerdefIterator &b)
    {
      return a.AtEnd != b.AtEnd;
    }

    /** True if the iterator is past the last version definition.  */
    bool AtEnd;

    /** Current version definition.  */
    ElfVerdef Verdef;
  };

  /** First element of the Section list.  */
  VerdefIterator verdef_begin(void)
  {
    return VerdefIterator(*this);
  }

  /** Invalid element of the Section list.  */
  VerdefIterator verdef_end(void)
  {
    return VerdefIterator(*this, nullptr);
  }

  protected:
  /** Reference to the ELF object (file).  */
  ElfObject &ElfObj;

  /** Header of this section.  */
  GElf_Shdr SectionHeader;

  /** Cached Elf_Data object.  */
  Elf_Data *Data;

  /** Section object from libelf.  */
  Elf_Scn *Section;
};

inline ElfVerdef::ElfVerdef(ElfSection &section)
  : Section(section), Offset(0)
{
  gelf_getverdef(Section.Get_Data(), Offset, &Verdef);
}

inline bool ElfVerdef::To_Next_Verdef(void)
{
  if (!Have_Next_Verdef())
    return false;

  Offset += Verdef.vd_next;
  return gelf_getverdef(Section.Get_Data(), Offset, &Verdef) != nullptr;
}

/** @brief ELF object.
  *
  * This class wraps the `struct Elf` object from libelf, also carring the file
  * descriptor used to open the file.  It provies automatic destructor so that
  * there is no need to close the fd or destroy the ELF object manually.
  */
class ElfObject : public Parser
{
  public:
  /** Open ELF object.  */
  ElfObject(const char *path);
  inline ElfObject(const std::string &path)
    : ElfObject(path.c_str())
  {
  }

  ~ElfObject(void);

  /** Get original `struct Elf` from libelf.  */
  inline Elf *Get_Wrapped_Object(void)
  {
    return ElfObj;
  }

  Elf *decompress_gz(void);
  Elf *decompress_zstd(void);

  /** Map from symbol version index to version name.  */
  typedef std::unordered_map<unsigned, std::string> VersionNameHash;

  /** Build a map from symbol version index to version name.  */
  VersionNameHash Get_Version_Names(void);

  /** Get the data of the symbol version section (SHT_GNU_versym), or nullptr
      if the object has no symbol versioning.  */
  Elf_Data *Get_Versym_Data(void);

  /** Iterator class for ELF sections.  With this one can use C++ iterators
    * to iterate through all sections of the ELF file.  Like this:
    * ```
    * for (auto it = EO.section_begin(); it != EO.section_end(); ++it)
    * ```
    **/

  struct SectionIterator
  {
    public:
    SectionIterator(ElfObject &eo, Elf_Scn *scn)
      : ElfScn(eo, scn)
    {
    }

    SectionIterator(ElfSection section)
      : ElfScn(section)
    {
    }

    SectionIterator(ElfObject &eo)
      : SectionIterator(eo, elf_nextscn(eo.Get_Wrapped_Object(), nullptr))
    {
    }

    /** Doing `*it` should access the Elf Section object.  */
    ElfSection &operator*()
    {
      return ElfScn;
    }

    /** Doing `*it` should access the Elf Section object.  */
    const ElfSection &operator*() const
    {
      return ElfScn;
    }

    /** Prefix iterator (++it).  */
    SectionIterator &operator++(void)
    {
      ElfScn.To_Next_Section();
      return *this;
    }

    /** Comparison must compare the underlying Section objects.  */
    inline friend bool operator==(const SectionIterator &a, const SectionIterator &b)
    {
      return a.ElfScn == b.ElfScn;
    }

    /** Comparison must compare the underlying Section objects.  */
    inline friend bool operator!=(const SectionIterator &a, const SectionIterator &b)
    {
      return a.ElfScn != b.ElfScn;
    }

    private:
    /** Wrap the ElfSection class.  */
    ElfSection ElfScn;
  };

  /** First element of the Section list.  */
  SectionIterator section_begin(void)
  {
    return SectionIterator(*this);
  }

  /** Invalid element of the Section list.  */
  SectionIterator section_end(void)
  {
    return SectionIterator(*this, nullptr);
  }

  private:
  /** Wrapped libelf object.  */
  struct Elf *ElfObj;

  /* Memory allocated to hold the decompressed ELF images */
  unsigned char *DecompressedObj;

  /** File descriptor used by libelf object.  */
  int ElfFd;
};

/** ElfSymbolCache -- Cache ELF symbols content for fast access.
  *
  * Very often we have to check if a given symbol is available in the debuginfo
  * file.  This is a fast way of doing this instead of reparse the ELF
  * structure.
  **/
class ElfSymbolCache
{
  public:
  /** Build cache from ElfObject).  */
  ElfSymbolCache(ElfObject &eo);

  /** Build empty cache to be filled later.  */
  ElfSymbolCache(void)
    : DynsymMap(),
      SymtabMap(),
      VersionNames(),
      Mod(""),
      DebuginfoPath(""),
      ObjectPath("")
  {};

  /** Analyze ELF object.  */
  void Analyze_ELF(ElfObject &eo);

  /* Get symbol if available in the Dynsym table.  Or 0 if not available.  */
  inline unsigned char Get_Symbol_Info_Dynsym(const std::string &sym)
  {
    auto container = DynsymMap.find(sym);
    if (container != DynsymMap.end()) {
      return container->second;
    }
    return 0;
  }

  /** Get symbol if available in Symtab table.  Or 0 if not available.  */
  inline unsigned char Get_Symbol_Info_Symtab(const std::string &sym)
  {
    auto container = SymtabMap.find(sym);
    if (container != SymtabMap.end()) {
      return container->second;
    }
    return 0;
  }

  /** Get symbol if available in either symtab.  Returns in which symtab this
      symbo was found.  */
  std::pair<unsigned char, ElfSymtabType> Get_Symbol_Info(const std::string &sym);

  std::string Get_Symbol_Module(const std::string &)
  {
    return Mod;
  }

  std::vector<std::string> Get_All_Symbols(void);

  inline const std::string &Get_Debuginfo_Path(void) const
  {
    return DebuginfoPath;
  }

  inline const std::string &Get_Object_Path(void) const
  {
    return ObjectPath;
  }

  /** Dump for debugging reasons.  */
  void Dump_Cache(void);

  private:

  /** The hashes.  */
  typedef std::unordered_map<std::string, unsigned char> SymbolTableHash;
  typedef ElfObject::VersionNameHash VersionNameHash;

  void Insert_Symbols_Into_Hash(SymbolTableHash &map, ElfSection &section);

  /** Insert the symbols of the .dynsym section.  Symbols with a private
      version (e.g. GLIBC_PRIVATE) can't be referenced directly by other
      objects, hence they are inserted into the symtab map instead.  */
  void Insert_Dynsym_Symbols(ElfObject &eo, ElfSection &dynsym);

  /** Check if the symbol at `index` in the dynsym has a private version.  */
  bool Is_Private_Symbol(Elf_Data *versym_data, size_t index);

  /** Hash symbols from dynsym into their value.  */
  SymbolTableHash DynsymMap;

  /** Hash symbols from symtab into their value.  */
  SymbolTableHash SymtabMap;

  /** Hash from symbol version index to version name of the dynsym.  */
  VersionNameHash VersionNames;

  /** Kernel module name, if .modinfo section is present. */
  std::string Mod;

  /** Path to the debuginfo object (contains .symtab section).  */
  std::string DebuginfoPath;

  /** Path to the .so file object (contains .dynsym section).  */
  std::string ObjectPath;
};
