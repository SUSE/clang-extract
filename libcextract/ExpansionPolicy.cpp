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
  // absolute_path and relative_path are different when extracting code from a
  // compiled kernel directory, so we need to check whether the include paths
  // start from include paths.
  if (absolute_path != relative_path)
    return !relative_path.starts_with("./include");

  std::vector<std::string> include_paths = { "/include/", "/arch/" };

  for (auto &path : include_paths) {
    if (absolute_path.find(path) != std::string::npos)
      return false;
  }

  // The subpath wasn't found, so it's not a public header. In this case we
  // should expand it.
  return true;
}

IncludeExpansionPolicy *IncludeExpansionPolicy::Get_Expansion_Policy(
                                                IncludeExpansionPolicy::Policy p)
{
  switch (p) {
    case Policy::NOTHING:
      return new NoIncludeExpansionPolicy();
      break;

    case Policy::EVERYTHING:
      return new ExpandEverythingExpansionPolicy();
      break;

    case Policy::KERNEL:
      return new KernelExpansionPolicy();
      break;

    default:
      assert(false && "Invalid policy");
  }
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
    { "kernel",     IncludeExpansionPolicy::KERNEL     }
  };

  for (unsigned long i = 0; i < ARRAY_LENGTH(policies); i++) {
    if (strcmp(str, policies[i].string) == 0) {
      return policies[i].policy;
    }
  }

  return IncludeExpansionPolicy::NOTHING;
}
