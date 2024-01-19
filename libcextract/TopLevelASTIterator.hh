//===- TopLevelASTIterator.cpp - Iterate through toplevel entities *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Iterate through toplevel entities: both macros and decls.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#pragma once

#include "MacroWalker.hh"
#include "clang/Frontend/ASTUnit.h"

#include <vector>

using namespace clang;

class TopLevelASTIterator
{
  public:
  TopLevelASTIterator(ASTUnit *ast, bool skip_macros_in_decls=true);

  enum ReturnType
  {
    TYPE_INVALID,
    TYPE_DECL,
    TYPE_PREPROCESSED_ENTITY,
    TYPE_MACRO_UNDEF,
  };

  struct Return
  {
    Return(Decl *d)
    {
      AsDecl = d;
      Type = ReturnType::TYPE_DECL;
    }

    Return(PreprocessedEntity *p)
    {
      AsPrep = p;
      Type = ReturnType::TYPE_PREPROCESSED_ENTITY;
    }

    Return(MacroDirective *u)
    {
      AsUndef = u;
      Type = ReturnType::TYPE_MACRO_UNDEF;
    }

    Return(void)
    {
      AsDecl = nullptr;
      Type = ReturnType::TYPE_INVALID;
    }

    union
    {
      Decl *AsDecl;
      PreprocessedEntity *AsPrep;
      MacroDirective *AsUndef;
    };
    ReturnType Type;

    SourceLocation Get_Location(void);

  } Current;

  inline struct Return& Get_Current(void)
  {
    return Current;
  }

  bool Advance(void);
  bool Skip_Until(const SourceLocation &loc);

  inline bool Is_Current_A_Decl()
  {
    return Current.Type == ReturnType::TYPE_DECL;
  }

  inline bool Is_Current_A_Preprocessed_Entity()
  {
    return Current.Type == ReturnType::TYPE_PREPROCESSED_ENTITY;
  }

  inline bool Is_Current_A_Macro_Undef()
  {
    return Current.Type == ReturnType::TYPE_MACRO_UNDEF;
  }

  /** Doing `*it` should access the Current object.  */
  struct Return &operator*()
  {
    return Current;
  }

  const struct Return &operator*() const
  {
    return Current;
  }

  /** Prefix iterator (++it) should advance to next element.  */
  TopLevelASTIterator &operator++(void)
  {
    Ended = !Advance();
    return *this;
  }

  bool End(void)
  {
    return Ended;
  }

  private:
  void Populate_Needs_Undef(void);

  ASTUnit *AST;
  SourceManager &SM;
  PreprocessingRecord &PrepRec;

  /* AST iterator.  */
  ASTUnit::top_level_iterator DeclIt;

  /* Macro Iterators.  */
  PreprocessingRecord::iterator MacroIt;
  unsigned UndefIt;

  /* Vector of macros that needs to be undeclared.  */
  std::vector<MacroDirective*> NeedsUndef;

  /* Comparator for SourceLocations.  */
  BeforeThanCompare<SourceLocation> BeforeClass;
  inline bool Is_Before(const SourceLocation &a, const SourceLocation &b)
  {
    assert(a.isValid());
    assert(b.isValid());
    return BeforeClass(a, b);
  }

  inline bool Is_After(const SourceLocation &a, const SourceLocation &b)
  {
    assert(a.isValid());
    assert(b.isValid());
    return BeforeClass(b, a);
  }

  MacroWalker MW;

  bool SkipMacrosInDecls;
  bool Ended;
  SourceLocation EndLocOfLastDecl;

  public:
  void Debug_Print(void);
};
