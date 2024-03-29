//===- SymversParser.cpp - Parse linux Modules.symvers files ---*- C++ --*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Parse linux Modules.symvers files.
//
//===----------------------------------------------------------------------===//

/** SymversParse
 *
 * The Module.symvers file contains all exported functions from the kernel
 * (vmlinux) and module that were built with the kernel. Parsing these functions
 * helps to check whether a symbol needs to be externalized or not.
 *
 * In the case of a symbol that exists in a different module, we need to
 * externalize it, since a kernel livepatch module cannot depend on another
 * modules.
 *
 * If the symbol resides in the vmlinux, we don't need to externalize it, since
 * the symbol will always be there.
 *
 * Author: Marcos Paulo de Souza
 */

#pragma once

#include "Parser.hh"

#include <cassert>
#include <string>
#include <iterator>
#include <unordered_map>

/** @brief Kernel symbol
  *
  * The symbol is question is always related to a module, or to vmlinux
  */
class Symbol
{
  public:
  Symbol(const std::string &name, const std::string &mod)
    : name(name),
      mod(mod)
  {
    // Ensure that we have a Symbol name
    assert(!name.empty());
  }

  std::string GetName()
  {
    return name;
  }

  const std::string &GetName() const
  {
    return name;
  }

  std::string GetModule()
  {
    return mod;
  }

  const std::string &GetModule() const
  {
    return mod;
  }

  private:
  std::string name;
  std::string mod;
};

/** Symvers -- Parse and cache Module.symvers symbols content for fast access.
  *
  * Cache the symbols and modules found for fast access.
  **/
class Symvers : public Parser
{
  public:
  Symvers(const std::string &path);

  void Parse();

  /* Get symbol if available in the Symvers file.  Or 0 if not available.  */
  inline bool Symbol_Exists(const std::string &sym)
  {
    auto container = map.find(sym);
    return container != map.end();
  }

  std::vector<std::string> Get_All_Symbols()
  {
    std::vector<std::string> vec;

    for (auto sym : map)
      vec.push_back(sym.first);

    return vec;
  }

  /* Get symbol if available in the Symvers file.  Or 0 if not available.  */
  std::string Get_Symbol_Module(const std::string &sym)
  {
    auto container = map.find(sym);
    // Symbol not found
    if (container == map.end())
      return {};
    return container->second;
  }

  // Externalize the symbols that are not part of vmlinux
  bool Needs_Externalization(const std::string &sym_mod)
  {
    return sym_mod != "vmlinux";
  }

  /* Cache the symbols from a module.  */
  inline void Insert_Symbols_Into_Hash(const Symbol &sym)
  {
    map[sym.GetName()] = sym.GetModule();
  }

  /** Dump for debugging reasons.  */
  void Dump(void);

  /** Dump for debugging reasons.  */
  void Dump_Cache(void);

  private:
  /** Hash symbols from symtab into their value.  */
  std::unordered_map<std::string, std::string> map;
};
