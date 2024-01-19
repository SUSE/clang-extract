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

/*
 *  clang-extract - Extract functions from projects and its dependencies using
 *                  libclang and LLVM infrastructure.
 *
 *  Copyright (C) 2024 SUSE Software Solutions GmbH
 *
 *  This file is part of clang-extract.
 *
 *  clang-extract is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  clang-extract is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with clang-extract.  If not, see <http://www.gnu.org/licenses/>.
 */

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
