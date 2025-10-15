//===- ExpansionPolicy.cpp - Declare header expansion policy --- *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Implements a Expansion Policy for #include headers.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi, Marcos Paulo de Souza.  */

#include "ExpansionPolicy.hh"
#include "NonLLVMMisc.hh"

#include <llvm/Support/raw_ostream.h>

bool KernelExpansionPolicy::Must_Expand(const StringRef &absolute_path,
                                        const StringRef &relative_path)
{
  std::vector<std::string> include_paths = { "/include/", "/arch/" };

  for (auto &path : include_paths) {
    if (absolute_path.find(path) != std::string::npos)
      return false;
  }

  // The subpath wasn't found, so it's not a public header. In this case we
  // should expand it.
  return true;
}

bool SystemExpansionPolicy::Must_Expand(const StringRef &absolute_path,
                                        const StringRef &relative_path)
{
  const char *absolute_path_c = absolute_path.data();

  /* Look for system headers by looking to specific prefixes.  */
  const char *include_paths[] = { "/usr/include/", "/usr/lib64/",
                                  "/usr/lib/", "/usr/local/include/", };
  for (unsigned i = 0; i < ARRAY_LENGTH(include_paths); i++) {
    if (prefix(include_paths[i], absolute_path_c)) {
      return false; // Do not expand.
    }
  }

  /* Expand anything that doesn't match this.  */
  return true;
}

bool CompilerExpansionPolicy::Must_Expand(const StringRef &absolute_path,
                                          const StringRef &relative_path)
{
  const char *absolute_path_c = absolute_path.data();

  /* Look for clang compiler headers by looking to specific prefixes.  */
  const char *include_paths[] = { "/usr/lib64/clang/", "/usr/lib/clang/",
                                  "/usr/local/lib64/clang/", "/usr/local/lib64/clang/", };
  for (unsigned i = 0; i < ARRAY_LENGTH(include_paths); i++) {
    if (prefix(include_paths[i], absolute_path_c)) {
      return false; // Do not expand.
    }
  }

  /* Expand anything that doesn't match this.  */
  return true;
}

bool CompilerExpansionPolicy::Must_Not_Expand(const StringRef &absolute_path,
                                              const StringRef &relative_path)
{
  return !Must_Expand(absolute_path, relative_path);
}

std::unique_ptr<IncludeExpansionPolicy> IncludeExpansionPolicy::Get_Expansion_Policy_Unique(
                                                        IncludeExpansionPolicy::Policy p)
{
  switch (p) {
    case Policy::NOTHING:
      return std::make_unique<NoIncludeExpansionPolicy>(
                NoIncludeExpansionPolicy());
      break;

    case Policy::EVERYTHING:
      return std::make_unique<ExpandEverythingExpansionPolicy>(
                ExpandEverythingExpansionPolicy());
      break;

    case Policy::KERNEL:
      return std::make_unique<KernelExpansionPolicy>(
                KernelExpansionPolicy());
      break;

    case Policy::SYSTEM:
      return std::make_unique<SystemExpansionPolicy>(
                SystemExpansionPolicy());
      break;

    case Policy::COMPILER:
      return std::make_unique<CompilerExpansionPolicy>(
                CompilerExpansionPolicy());
      break;

    default:
      assert(false && "Invalid policy");
  }
}

IncludeExpansionPolicy::Policy IncludeExpansionPolicy::Get_From_String(const char *str)
{
  if (str == nullptr) {
    return IncludeExpansionPolicy::INVALID;
  }

  struct {
    const char *string;
    Policy policy;
  } policies[] = {
    { "nothing",    IncludeExpansionPolicy::NOTHING    },
    { "everything", IncludeExpansionPolicy::EVERYTHING },
    { "kernel",     IncludeExpansionPolicy::KERNEL     },
    { "system",     IncludeExpansionPolicy::SYSTEM     },
    { "compiler",   IncludeExpansionPolicy::COMPILER   },
  };

  for (unsigned long i = 0; i < ARRAY_LENGTH(policies); i++) {
    if (strcmp(str, policies[i].string) == 0) {
      return policies[i].policy;
    }
  }

  return IncludeExpansionPolicy::NOTHING;
}


/* Return true if headers passed through -include must be expanded.  */
bool IncludeExpansionPolicy::Expand_Minus_Includes(IncludeExpansionPolicy::Policy policy)
{
  switch (policy) {
    case INVALID:
    case EVERYTHING:
    case SYSTEM:
    case COMPILER:
      return true;
      break;

    case NOTHING:
    case KERNEL:
      return false;
      break;
  }
}
