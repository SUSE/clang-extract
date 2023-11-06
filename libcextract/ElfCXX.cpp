#include "ElfCXX.hh"
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdexcept>
#include <iostream>

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
  ElfFd = open(parser_path, O_RDONLY);
  if (ElfFd == -1) {
    throw std::runtime_error("File not found!");
  }

  /* Create libelf object and store it in the class variable.  */
  ElfObj = elf_begin(ElfFd, ELF_C_READ, nullptr);
  if (ElfObj == nullptr) {
    close(ElfFd);
    throw std::runtime_error("libelf error: "
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

      default:
        break;
    }
  }
}

std::vector<std::string> ElfSymbolCache::Get_All_Symbols(void)
{
  std::vector<std::string> vec;

  for (auto it = EO.section_begin(); it != EO.section_end(); ++it) {
    ElfSection &section = *it;
    if (section.Is_Symbol_Table()) {
      unsigned n = section.Get_Num_Symbols();
      for (unsigned i = 0; i < n; i++) {
        ElfSymbol sym = section.Get_Symbol(i);
        unsigned char type = sym.Get_Type();
        if (type == STT_FUNC || type == STT_OBJECT) {
          const char *name = sym.Get_Name();
          vec.push_back(std::string(name));
        }
      }
    }
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
