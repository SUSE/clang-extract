//===- ExpansionPolicy.hh - Declare header expansion policy ---- *- C++ -*-===//
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

/* Author: Giuliano Belinassi  */

#pragma once

#include <llvm/ADT/StringRef.h>
#include <memory>

using namespace llvm;

class IncludeExpansionPolicy
{
  public:
  virtual bool Must_Expand(const StringRef &absolute_path, const StringRef &relative_path) = 0;

  virtual bool Must_Not_Expand(const StringRef &absolute_path, const StringRef &relative_path) = 0;


  virtual ~IncludeExpansionPolicy() = default;

  enum Policy {
    INVALID,
    NOTHING,
    EVERYTHING,
    KERNEL,
    SYSTEM,
    COMPILER,
  };

  static std::unique_ptr<IncludeExpansionPolicy>
                Get_Expansion_Policy_Unique(Policy policy);

  /* Return if headers passed through -include must be expanded.  */
  static bool Expand_Minus_Includes(Policy policy);

  static Policy Get_From_String(const char *string);
  inline static Policy Get_Overriding(const char *string, bool is_kernel)
  {
    /* In case a policy name wasn't specified (string is null), then override
       acoriding to the following guess.  */
    Policy p = Get_From_String(string);
    if (p == Policy::INVALID) {
      if (is_kernel) {
        return Policy::KERNEL;
      } else {
        return Policy::NOTHING;
      }
    }
    return p;
  }
  protected:
};

class NoIncludeExpansionPolicy : public IncludeExpansionPolicy
{
  public:
  virtual bool Must_Expand(const StringRef &absolute_path, const StringRef &relative_path)
  {
    return false;
  }

  virtual bool Must_Not_Expand(const StringRef &absolute_path, const StringRef &relative_path)
  {
    return false;
  }

};

class ExpandEverythingExpansionPolicy : public IncludeExpansionPolicy
{
  public:
  virtual bool Must_Expand(const StringRef &absolute_path, const StringRef &relative_path)
  {
    return true;
  }

  virtual bool Must_Not_Expand(const StringRef &absolute_path, const StringRef &relative_path)
  {
    return false;
  }
};

/** Expand any header according to kernel livepatching rules.  */
class KernelExpansionPolicy : public IncludeExpansionPolicy
{
  public:
  virtual bool Must_Expand(const StringRef &absolute_path, const StringRef &relative_path);

  virtual bool Must_Not_Expand(const StringRef &absolute_path, const StringRef &relative_path)
  {
    return false;
  }
};

/** Expand any header that is not installed in the system.  */
class SystemExpansionPolicy : public IncludeExpansionPolicy
{
  public:
  virtual bool Must_Expand(const StringRef &absolute_path, const StringRef &relative_path);

  virtual bool Must_Not_Expand(const StringRef &absolute_path, const StringRef &relative_path)
  {
    return false;
  }
};

/** Expand any header that is not compiler-specific.  */
class CompilerExpansionPolicy : public IncludeExpansionPolicy
{
  public:
  virtual bool Must_Expand(const StringRef &absolute_path, const StringRef &relative_path);
  virtual bool Must_Not_Expand(const StringRef &absolute_path, const StringRef &relative_path);
};
