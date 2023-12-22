#pragma once

#include "MacroWalker.hh"
#include "InlineAnalysis.hh"

#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>

using namespace clang;

struct ExternalizerLogEntry
{
  std::string OldName;
  std::string NewName;
  ExternalizationType Type;
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

  /* Check if two changes results in the same change.  */
  bool Is_Same_Change(const Delta &a, const Delta &b);

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
  SymbolExternalizer(ASTUnit *ast, InlineAnalysis &ia, bool dump = false)
    : AST(ast),
      MW(ast->getPreprocessor()),
      TM(ast, dump),
      IA(ia)
  {
  }

  class FunctionUpdater
  {
    public:
    /** A reference to SymbolExternalizer.  */
    SymbolExternalizer &SE;

    /** The new variable declaration to replace the to be externalized function.  */
    ValueDecl *NewSymbolDecl;

    /** Name of the to be replaced function.  */
    const std::string &OldSymbolName;

    FunctionUpdater(SymbolExternalizer &se, ValueDecl *new_decl,
                    const std::string &old_decl_name, bool wrap)
    : SE(se),
      NewSymbolDecl(new_decl),
      OldSymbolName(old_decl_name),
      Wrap(wrap)
    {}

    /** Sweeps the function and update any reference to the old function, replacing
        it with the externalized variable.  */
    bool Update_References_To_Symbol(DeclaratorDecl *to_update);

    private:

    /* Decl to update.  */
    DeclaratorDecl *ToUpdate;

    /* Do we need to wrap the use in (*name)?  */
    bool Wrap;

    bool Update_References_To_Symbol(Stmt *);
  };
  friend class FunctionUpdater;

  /* Create the externalized var as a AST node ready to be slapped into the
     AST.  */
  VarDecl *Create_Externalized_Var(DeclaratorDecl *decl, const std::string &name);

  /** Externalize a symbol, that means transforming functions into a function
      pointer, or an global variable into a variable pointer.  */
  void Externalize_Symbol(DeclaratorDecl *to_externalize);
  void Externalize_Symbol(const std::string &to_externalize);
  void Externalize_Symbols(std::vector<std::string> const &to_externalize_array);

  /* WARNING: Modifies the given vector.  */
  void Rename_Symbols(std::vector<std::string> &to_rename_array);

  bool _Externalize_Symbol(const std::string &to_externalize, ExternalizationType type);

  /* Drop `static` keyword in decl.  */
  bool Drop_Static(FunctionDecl *decl);

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

  private:

  void Strongly_Externalize_Symbol(const std::string &to_externalize);
  void Weakly_Externalize_Symbol(const std::string &to_externalize);
  void _Externalize_Symbol(const std::string &to_externalize);

  void Rewrite_Macros(std::string const &to_look_for, std::string const &replace_with);

  /* Issue a Text Replacement with a given `priority`.  The priority will be
     used in case that there are two replacements to the same piece of text.  */
  void Replace_Text(const SourceRange &range, StringRef new_name, int priority);
  void Remove_Text(const SourceRange &range, int priority);

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
};
