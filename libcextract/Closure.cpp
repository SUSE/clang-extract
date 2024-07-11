//===- Closure.hh - Compute dependencies of a given symbol *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Compute a set of Decls that is necessary to compile a given symbol.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#include "Closure.hh"

/** Add a decl to the Dependencies set and all its previous declarations in the
    AST. A function can have multiple definitions but its body may only be
    defined later.  */
bool ClosureSet::Add_Decl_And_Prevs(Decl *decl)
{
  bool inserted = false;

  /* Do not insert builtin decls.  */
  if (Is_Builtin_Decl(decl)) {
    return false;
  }

  while (decl) {
    if (!Is_Decl_Marked(decl)) {
      Dependencies.insert(decl);
      inserted = true;
    }

    decl = decl->getPreviousDecl();
  }
  return inserted;
}

void DeclClosureVisitor::Compute_Closure_Of_Symbols(const std::vector<std::string> &names,
                                          std::unordered_set<std::string> *matched_names)
{
  /* FIXME: clang has a mechanism (DeclContext::lookup) which is SUPPOSED TO
     return the list of all Decls that matches the lookup name.  However, this
     method doesn't work as intented.  In kernel (see github issue #20) it
     results in the lookup_result set not having the Decl that has the body of
     the function, which is the most important!

     Hence, do our own thing here.  Create a set and sweep the top level decls,
     analyzing the symbol that matches the name in the set.  */

  std::unordered_set<std::string> setof_names(std::make_move_iterator(names.begin()),
                                              std::make_move_iterator(names.end()));

  for (auto it = AST->top_level_begin(); it != AST->top_level_end(); ++it) {
    NamedDecl *decl = dyn_cast<NamedDecl>(*it);

    if (!decl) {
      /* Decl does not have a name, thus skip it.  */
      continue;
    }

    const std::string &decl_name = decl->getName().str();
    /* If the symbol name is in the set...   */
    if (setof_names.find(decl_name) != setof_names.end()) {
      /* Mark that name as matched.  */
      if (matched_names)
        matched_names->insert(decl_name);
      /* Find its dependencies.  */
      TraverseDecl(decl);
    }
  }
}
