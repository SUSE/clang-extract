//===- SymbolExternalizer.cpp - Externalize or rename symbols ---*- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Externalize or rename symbols.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#pragma once

#include "MacroWalker.hh"
#include "InlineAnalysis.hh"
#include "Closure.hh"

#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include "llvm/ADT/StringMap.h"

using namespace clang;

struct ExternalizerLogEntry
{
  std::string OldName;
  std::string NewName;
  ExternalizationType Type;
};

/* Contains the context of an externalized symbol. It's necessary when the
 * symbol is being handled in different visitors to know why/if it was already
 * externalized. */
struct SymbolUpdateStatus
{

  SymbolUpdateStatus(std::string new_name, ExternalizationType ext, bool done, bool wrap) :
    NewName(new_name),
    ExtType(ext),
    OldDecl(nullptr),
    NewDecl(nullptr),
    FirstUse(nullptr),
    LateInsertLocation(SourceLocation()),
    Done(done),
    Wrap(wrap)
  {
  }

  SymbolUpdateStatus(ExternalizationType ext) :
    NewName(""),
    ExtType(ext),
    OldDecl(nullptr),
    NewDecl(nullptr),
    FirstUse(nullptr),
    LateInsertLocation(SourceLocation()),
    Done(false),
    Wrap(false)
  {
  }

  /* The name that was used when the symbol was externalized. */
  std::string NewName;

  /* The type of the externalization: STRONG, WEAK of RENAME. */
  ExternalizationType ExtType;

  /* The old non-externalized declaration.  */
  DeclaratorDecl *OldDecl;

  /* The new externalized declaration.  */
  DeclaratorDecl *NewDecl;

  /* The first use of the symbol.  */
  DeclRefExpr *FirstUse;

  /* The late insertion location.  */
  SourceLocation LateInsertLocation;

  /* Set when the symbol was externalized. */
  bool Done;

  /* Set when the symbol usage should be dereferenced when used. */
  bool Wrap;

  /* Get the correct name used when the code was transformed */
  inline std::string getUseName(void)
  {
      return Wrap ? "(*" + NewName + ")"
                  : NewName;
  }

  /* Check if this SymbolUpdateStatus was used, e.g. it wasn't a result of an
     attempt of externalize an symbol which was removed by the ClosurePass.  */
  inline bool Is_Used(void)
  {
    return OldDecl && NewDecl && FirstUse;
  }

  /* For IBT, NewDecl and OldDecl are the same if the symbols is a function, so
     don't replace text if the name didn't change. */
  inline bool Needs_Sym_Rename(void) {
    if (NewDecl == nullptr)
      return true;

    return NewDecl->getName() != OldDecl->getName();
    /*
    llvm::outs() << "XX OldName" << "\n";
    llvm::outs() << OldDecl->getName() << "\n";
    llvm::outs() << "XX NewName print" << "\n";
    NewDecl->print(llvm::outs(), 0);
    llvm::outs() << "XX NewName name" << "\n";
    llvm::outs() << NewDecl->getName() << "\n";
    */
  }

  void Dump(SourceManager &SM)
  {
    llvm::outs() << "SymbolUpdateStatus at 0x" << this << '\n';
    if (OldDecl != nullptr) {
      SourceLocation loc = OldDecl->getLocation();
      PresumedLoc ploc = SM.getPresumedLoc(loc);
      llvm::outs() << "  OldDecl " << OldDecl->getName() << " at " << ploc.getFilename() << ':' << ploc.getLine() << '\n';
    } else {
      llvm::outs() << "  OldDecl is NULL\n";
    }
    if (FirstUse != nullptr) {
      SourceLocation loc = FirstUse->getLocation();
      PresumedLoc ploc = SM.getPresumedLoc(loc);
      llvm::outs() << "  FirstUse at " << ploc.getFilename() << ':' << ploc.getLine() << ':' << ploc.getColumn() << '\n';
    } else {
      llvm::outs() << "  FirstUse is NULL\n";
    }
    if (LateInsertLocation.isValid()) {
      PresumedLoc ploc = SM.getPresumedLoc(LateInsertLocation);
      llvm::outs() << "  LateInsertLocation at " << ploc.getFilename() << ':' << ploc.getLine() << ':' << ploc.getColumn() << '\n';
    } else {
      llvm::outs() << "  LateInsertLocation is invalid\n";
    }
    llvm::outs() << "  ExternalizationType = " << ExtType << '\n';
  }
};

/** Text Modification class wrapping Clang's Rewriter.
 *
 *  Clang has a Rewriter class in order to issue Text Modifications into the
 *  same Compilation Unit.  The problem is that this class expects that every
 *  modifications is commutative, that means A + B = B + A.  This is not the
 *  case when whe are externalizing things, once we may externalize a function
 *  that in its body contains a reference to another externalized function.
 *  Hence we have to cleverly find which modifications stays and which should
 *  be discarded.
 *
 *  We do this by setting a priority in each Text Modifications, and if two
 *  modifications wants to modify the same piece of text, we only let the one
 *  with highest priority.  With this, we classify the modifications which
 *  removes content to be the highest priority, and the ones which only changes
 *  the name of things to be the lowest priority.  Hence at some point we compute
 *  the insersection of all with all and discards the ones with lower priority.
 */
class TextModifications
{
  public:
  /** A delta -- a single text modification.  */
  struct Delta
  {
    Delta(const SourceRange &to_change, const std::string &new_text, int prio);

    Delta(void)
      : ToChange(),
        NewText(),
        Priority(0),
        ID(0)
    {
    }

    /* Check if two changes results in the same change.  */
    static bool Is_Same_Change(const Delta &a, const Delta &b)
    {
      return a.ToChange == b.ToChange && a.NewText == b.NewText;
    }

    bool operator<(const Delta& other) const
    {
      return !(Is_Same_Change(*this, other));
    }

    bool operator==(const Delta &other) const
    {
      return Is_Same_Change(*this, other);
    }

    /* Which part of the original code should be changed?  */
    SourceRange ToChange;

    /* With what text?  */
    std::string NewText;

    /* What is the priority of this change?  */
    int Priority;

    /* ID.  */
    int ID;
  };

  /** Constructor for the TextModification class.  */
  TextModifications(ASTUnit *ast, bool dump = false)
    : SM(ast->getSourceManager()),
      LO(ast->getLangOpts()),
      RW(SM, LO),
      DumpingEnabled(dump)
  {}

  /** Get underlying Rewriter class.  */
  inline Rewriter &Get_Rewriter(void)
  {
    return RW;
  }

  /* Insert a text modification.  */
  void Insert(Delta &delta);

  /* Solve the modifications according to their priorities and apply to clang's
     Rewriter class instance.  */
  void Commit(void);

  /* Get FileEntry map.  */
  inline const std::unordered_map<const FileEntry *, FileID> &Get_FileEntry_Map(void)
  {
    return FileEntryMap;
  }

  /* Insert into the FileEntryMap hash.  */
  bool Insert_Into_FileEntryMap(const SourceLocation &loc);
  bool Insert_Into_FileEntryMap(const SourceRange &range);

  /* Dump, for debugging purposes.  */
  void Dump(unsigned, const Delta &a);

  private:

  /* Solve the modifications according to their priorities.  */
  void Solve(void);

  /* Sort the DeltaLists (sequence of text modifications) according to their
     priorities.  */
  void Sort(void);

  /* Checks if two SourceRanges intersects.  */
  bool Intersects(const SourceRange &a, const SourceRange &b);

  /* Check if two Deltas (text modifications) intersects.  */
  bool Intersects(const Delta &a, const Delta &b);

  /* Check which SourceLocation comes first.  */
  bool Is_Before_Or_Equal(const PresumedLoc &a, const PresumedLoc &b);

  /* Reference to the AST SourceManager.  */
  SourceManager &SM;

  /* Reference to the AST LangOptions.  */
  const LangOptions &LO;

  /* The clang's Rewriter class.  */
  Rewriter RW;

  /* Flag indicating we want to dump our changes.  */
  bool DumpingEnabled;

  /* The list of Text Modifications we want to do.  */
  std::vector<Delta> DeltaList;

  /* Our own mapping from FileEntry to FileID to get the modifications to the
     files.  */
  std::unordered_map<const FileEntry *, FileID> FileEntryMap;
};

/** Class encapsulating the Symbol externalizer mechanism.
 *
 * Livepatched functions very often references static functions or variables
 * that we have to `externalize` them.  Assume we want to externalize a
 * function f. We externalize it by applying the following transformation:
 *
 * before:
 * int f(void) {
 *  ... code
 *  return x;
 * }
 *
 * after:
 * static int (*f)(void);
 *
 * then libpulp or klp is responsible for initializing the pointer `f` with
 * its correct address in the target binary.
 *
 */
class SymbolExternalizer
{
  public:
  SymbolExternalizer(ASTUnit *ast, InlineAnalysis &ia, bool ibt,
                     bool allow_late_externalize, std::string patch_object,
                     const std::vector<std::string> &functions_to_extract,
                     bool dump = false)
    : AST(ast),
      MW(ast->getPreprocessor()),
      TM(ast, dump),
      IA(ia),
      Ibt(ibt),
      AllowLateExternalization(allow_late_externalize),
      PatchObject(patch_object),
      SymbolsMap({}),
      ClosureVisitor(ast)
  {
    ClosureVisitor.Compute_Closure_Of_Symbols(functions_to_extract);
  }

  friend class ExternalizerVisitor;

  /* Create the externalized var as a AST node ready to be slapped into the
     AST.  */
  VarDecl *Create_Externalized_Var(DeclaratorDecl *decl, const std::string &name);

  /* Externalize a symbol, that means transforming functions into a function
     pointer, or an global variable into a variable pointer.

     WARNING: Modifies the to_rename_array vector. */
  void Externalize_Symbols(std::vector<std::string> const &to_externalize_array,
                          std::vector<std::string> &to_rename_array);
  inline void Externalize_Symbols(std::vector<std::string> const &to_externalize_array)
  {
    std::vector<std::string> empty = {};
    Externalize_Symbols(to_externalize_array, empty);
  }

  std::vector<std::pair<std::string, SourceRange>>
  Get_Range_Of_Identifier_In_Macro_Expansion(const MacroExpansion *exp);

  SymbolUpdateStatus *getSymbolsUpdateStatus(const StringRef &sym);

  /* Drop `static` keyword in decl.  */
  bool Drop_Static(FunctionDecl *decl);

  /* Replace `static` keyword in decl with extern.  */
  bool Add_Extern(FunctionDecl *decl);

  /** Commit changes to the loaded source file buffer.  Should NOT modify the
      original file, only the content that was loaded in llvm's InMemory file
      system.  */
  bool Commit_Changes_To_Source(IntrusiveRefCntPtr<llvm::vfs::OverlayFileSystem> &ofs,
                                IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> &mfs,
                                std::vector<std::string> &headers_to_expand);

  std::string Get_Modifications_To_Main_File(void);

  inline std::vector<ExternalizerLogEntry> &Get_Log_Of_Changed_Names(void)
  {
    return Log;
  }

  /** Returns true if there was at least one externalized symbol */
  inline bool Has_Externalizations(void)
  {
    for (auto const& [key, val] : SymbolsMap) {
      if (val.ExtType == ExternalizationType::STRONG)
        return true;
    }

    return false;
  }

  /** Dump the SymbolsMap structure into stdout for debugging purposes.  */
  void Dump_SymbolsMap(void);

  private:

  /** Get the ExternalizationType from a string.  */
  enum ExternalizationType Get_Symbol_Ext_Type(const std::string &to_externalize);

  /** Compute the late insert locations when -DCE_LATE_EXTERNALIZE is passed by
      the user.  It will attempt to find a SourceLocation before the first use
      of the variable that is valid, so we can put our externalized symbol there.  */
  void Compute_SymbolsMap_Late_Insert_Locations(std::vector<SymbolUpdateStatus *> &array);

  /** Do the late externalize logic.  */
  void Late_Externalize(void);

  /** Rewrite the macros which tokens matches the symbols we want to externalize.  */
  void Rewrite_Macros(void);

  /* Issue a Text Replacement with a given `priority`.  The priority will be
     used in case that there are two replacements to the same piece of text.  */
  void Replace_Text(const SourceRange &range, StringRef new_name, int priority);
  void Remove_Text(const SourceRange &range, int priority);
  void Insert_Text(const SourceLocation &range, StringRef text);

  /** AST in analysis.  */
  ASTUnit *AST;

  /** MacroWalker object which helps iterating on Macros.  */
  MacroWalker MW;

  /** Clang's Rewriter class used to auxiliate us with changes in the source code.  */
  TextModifications TM;

  /** Reference to the InlineAnalysis in the PassManager::Context instance.  */
  InlineAnalysis &IA;

  /** Log of changed names.  */
  std::vector<ExternalizerLogEntry> Log;

  /** Defines the method that a private symbol will be searched. */
  bool Ibt;

  /* True if we can write the externalized decl later than the original symbol.  */
  bool AllowLateExternalization;

  /* Name of the object that will be patched. */
  std::string PatchObject;

  /** Symbols and its externalization type */
  llvm::StringMap<SymbolUpdateStatus> SymbolsMap;

  /* ClosureVisitor to compute the closure.  */
  DeclClosureVisitor ClosureVisitor;
};
