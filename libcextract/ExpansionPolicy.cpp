#include "ExpansionPolicy.hh"
#include "NonLLVMMisc.hh"

#include <llvm/Support/raw_ostream.h>

bool KernelExpansionPolicy::Must_Expand(const StringRef &absolute_path,
                                        const StringRef &relative_path)
{
  /* Marcos, seu código vai aqui.  Essa classe é instanciada a cada vez que
     o FunctionDepsFinder é instanciado, ou seja, duas vezes no ClosurePass.

     Portanto guardar coisas na KernelExpansionPolicy será eventualmente
     descartado.
     */

  llvm::outs() << "abs: " << absolute_path << "  rel: " << relative_path << '\n';
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
