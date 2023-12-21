#include "SymversParser.hh"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <libgen.h>

Symvers::Symvers(const std::string &path)
    : Parser(path)
{
  Parse();
}

void Symvers::Parse()
{
  std::string line;
  std::ifstream f;
  f.open(parser_path);

  if (!f.is_open())
    throw std::runtime_error("File not found: " + parser_path);

  /* The Module.symvers file contains five fields separated by tabs:
   * https://www.kernel.org/doc/html/latest/kbuild/modules.html#symbols-from-the-kernel-vmlinux-modules
   * CRC	Symbol name	Module Path	Export type	Namespace
   *
   * All the symbols on Module.symvers are exported by the kernel and modules.
   * Any of these can be empty, namespace for example. At this point we only
   * care for the Symbol name and the module associates with it.
   */
  while (f.good()) {
    std::getline(f, line);

    /* Should be the last line */
    if (line.empty())
      continue;

    std::string temp;
    std::string sym_name;
    std::string sym_mod;

    std::stringstream ss(line);

    // Discard the CRC
    std::getline(ss, temp, '\t');
    // Get the symbol and module
    std::getline(ss, sym_name, '\t');
    std::getline(ss, sym_mod, '\t');

    // Only get the name of the module, instead of the path to it
    Symbol sym(sym_name, basename(sym_mod.data()));
    Insert_Symbols_Into_Hash(sym);
  }
}

void Symvers::Dump(void)
{
  std::cout << "Symbol\tModule" << std::endl;
  for (auto &p : map)
    std::cout << p.first << "\t" << p.second << std::endl;
}
