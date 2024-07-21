//===- ExpansionPolicy.cpp - Construct Closure of Symbols --- *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Implements the closure algorithm for symbols.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#include <sstream>
#include <vector>
#include "FunctionDepsFinder.hh"
#include "ClangCompat.hh"
#include "PrettyPrint.hh"
#include "Error.hh"
#include "LLVMMisc.hh"

/** FunctionDependencyFinder class methods implementation.  */
FunctionDependencyFinder::FunctionDependencyFinder(PassManager::Context *ctx)
    : AST(ctx->AST.get()),
      IT(AST, ctx->IncExpansionPolicy, ctx->HeadersToExpand),
      KeepIncludes(ctx->KeepIncludes),
      Visitor(AST)
{
}

bool FunctionDependencyFinder::Find_Functions_Required(
    std::vector<std::string> const &funcnames)
{
  std::unordered_set<std::string> matched_names;
  Visitor.Compute_Closure_Of_Symbols(funcnames, &matched_names);

  /* Find which names did not match any declaration name.  */
  for (const std::string &funcname : funcnames) {
    if (matched_names.find(funcname) == matched_names.end()) {
      DiagsClass::Emit_Error("Requested symbol not found: " + funcname);
      return false;
    }
  }

  return true;
}

void FunctionDependencyFinder::Print(void)
{
  ClosureSet &closure = Visitor.Get_Closure();
  RecursivePrint(AST, closure.Get_Set(), IT, KeepIncludes).Print();
}

void FunctionDependencyFinder::Remove_Redundant_Decls(void)
{
  ClosureSet &closure = Visitor.Get_Closure();
  std::unordered_set<Decl *> &closure_set = closure.Get_Set();

  for (auto it = closure_set.begin(); it != closure_set.end(); ++it) {
    /* Handle the case where a enum or struct is declared as:

        typedef enum Hand {
          LEFT,
          RIGHT
        } Hand;

        which is then broken as

        enum Hand {
          LEFT,
          RIGHT
        };

        typedef enum Hand Hand;

        but when printing, we track the typedef declaration to what the user
        wrote and it will be transformed into

        enum Hand {
          LEFT,
          RIGHT
        };

        typedef enum Hand {
          LEFT,
          RIGHT
        } Hand;

        which is a redeclaration of enum Hand. Hence we have to remove the
        first `enum Hand` from the closure.  See typedef-7.c testcase.  */
    if (TypedefDecl *decl = dyn_cast<TypedefDecl *>(*it)) {
      SourceRange range = decl->getSourceRange();

      const clang::Type *type = decl->getTypeForDecl();
      if (type) {
        TagDecl *typedecl = type->getAsTagDecl();

        if (typedecl && closure.Is_Decl_Marked(typedecl)) {
          SourceRange type_range = typedecl->getSourceRange();

          /* Check if the strings regarding an decl empty. In that case we
             can not delete the decl from list.  */
          if (PrettyPrint::Get_Source_Text(range) != "" &&
              PrettyPrint::Get_Source_Text(type_range) != "") {

            /* Using .fullyContains() fails in some declarations.  */
            if (PrettyPrint::Contains_From_LineCol(range, type_range)) {
              closure.Remove_Decl(typedecl);
            }
          }
        }
      }
    }
    /* Handle the case where an enum is declared as:
        extern enum system_states {
          SYSTEM_BOOTING,
          SYSTEM_SCHEDULING,
          SYSTEM_FREEING_INITMEM,
          SYSTEM_RUNNING,
          SYSTEM_HALT,
          SYSTEM_POWER_OFF,
          SYSTEM_RESTART,
          SYSTEM_SUSPEND,
        } system_state;

      In which will be broken down into:

        enum system_states {
          ...
        };

        enum system_states system_state;

        this will remove the first enum declaration because the location
        tracking will correctly include the enum system_states.  */

    else if (DeclaratorDecl *decl = dyn_cast<DeclaratorDecl *>(*it)) {
      SourceRange range = decl->getSourceRange();

      const clang::Type *type = ClangCompat::getTypePtr(decl->getType());
      /* There are some cases where a variable is declared as follows:

          static const struct mount_opts
          {
            int token;
            int mount_opts;
            int flags;
          } ext4_mount_opts[] = {
            {1, 1, 1}
          };

        in this case, the `ext4_mount_opts` array is declared together with its
        type, thus resulting in Clang splitting this into two decls in its AST:

          struct mount_opts
          {
            int token;
            int mount_opts;
            int flags;
          };

          static const struct ext4_mount_opts[] = {
            {1, 1, 1}
          };

        but since we try to get what the user wrote, it results in two declarations
        of `struct mount_opts`, thus clang-extract fails because of a redeclaration
        error. But since `ext4_mount_opts` is an array, geting its type returns
        an array type, not the struct declaration itself, hence check for this
        case as well.  */
      if (type->isArrayType() || type->isPointerType()) {
        type = type->getBaseElementTypeUnsafe();
      }

      TagDecl *typedecl = type ? type->getAsTagDecl() : nullptr;
      if (typedecl && closure.Is_Decl_Marked(typedecl)) {
        SourceRange type_range = typedecl->getSourceRange();

        /* Using .fullyContains() fails in some declarations.  */
        if (PrettyPrint::Contains_From_LineCol(range, type_range)) {
          closure.Remove_Decl(typedecl);
        }
      }
    }
  }

  // FIXME: use interval tree
  ASTUnit::top_level_iterator it;
  TypedefDecl *prev = nullptr;
  for (it = AST->top_level_begin(); it != AST->top_level_end(); ++it) {
    if (TypedefDecl *decl = dyn_cast<TypedefDecl>(*it)) {
      if (!closure.Is_Decl_Marked(decl))
        continue;

      // Set prev and exit, since we don't have anything to compare agains't
      if (!prev) {
        prev = decl;
        continue;
      }

      /*
       * Check if there wasn't any symbol that is being defined in the same
       * interval and remove it. Otherwise we might clash the types.
       *
       * One example of how this can happen is then we have something like
       *
       * typedef struct {
       * ...
       * } x, y;
       *
       * In the process of creating the closure we might reach the following
       * situation:
       *
       * typdef struct {
       * ...
       * } x;
       *
       * typedef struct {
       *
       * } x, y;
       *
       * Which then breaks the one-definition-rule. In such cases, remove the
       * previous declaration in the same code range, since the later will
       * contain both definitions either way.
       */
      if (PrettyPrint::Contains_From_LineCol(decl->getSourceRange(),
                                              prev->getSourceRange())) {
        /*
         * If the prev and the current decl have the same start LoC, but
         * different ending, remove the prev from the closure and set the
         * new prev.
         */
        closure.Remove_Decl(prev);
      }

      prev = decl;
    }
  }
}

void FunctionDependencyFinder::Insert_Decls_From_Non_Expanded_Includes(void)
{
  ASTUnit::top_level_iterator it;
  for (it = AST->top_level_begin(); it != AST->top_level_end(); ++it) {
    Decl *decl = *it;

    const SourceLocation &loc = decl->getLocation();
    IncludeNode *node = IT.Get(loc);

    if (node && node->Has_Parent_Marked_For_Output()) {
      Visitor.TraverseDecl(decl);
    }
  }
}

bool FunctionDependencyFinder::Run_Analysis(std::vector<std::string> const &functions)
{
  bool ret = true;
  /* Step 1: Compute the closure.  */
  ret &= Find_Functions_Required(functions);

  /* Step 2: Expand the closure to include Decls in non-expanded includes.  */
  if (KeepIncludes) {
    /* TODO: Guarding this with KeepIncludes should actually not be necessary,
       but we still have bugs when processing the kernel and the closure is not
       perfect.  By not adding those declarations we avoid hitting cases where
       it would hang because of a declaration that would be removed in the
       future anyway.  It also speeds up analysis considerably when the user do
       not want to keep the includes.  */
    Insert_Decls_From_Non_Expanded_Includes();
  }

  /* Step 3: Remove any declaration that may have been declared twice.  This
     is required because of the `enum-5.c` testcase (struct declared inside
     a typedef).  */
  Remove_Redundant_Decls();

  return ret;
}
