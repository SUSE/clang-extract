#pragma once

#include <llvm/ADT/StringRef.h>
#include <memory>

using namespace llvm;

class IncludeExpansionPolicy
{
  public:
  virtual bool Must_Expand(const StringRef &absolute_path, const StringRef &relative_path) = 0;

  virtual ~IncludeExpansionPolicy() = default;

  enum Policy {
    INVALID,
    NOTHING,
    EVERYTHING,
    KERNEL,
  };

  static IncludeExpansionPolicy *Get_Expansion_Policy(Policy policy);
  static std::unique_ptr<IncludeExpansionPolicy>
                Get_Expansion_Policy_Unique(Policy policy);

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

};

class ExpandEverythingExpansionPolicy : public IncludeExpansionPolicy
{
  public:
  virtual bool Must_Expand(const StringRef &absolute_path, const StringRef &relative_path)
  {
    return true;
  }
};

class KernelExpansionPolicy : public IncludeExpansionPolicy
{
  public:
  virtual bool Must_Expand(const StringRef &absolute_path, const StringRef &relative_path);
};
