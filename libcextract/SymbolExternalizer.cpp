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

#include "SymbolExternalizer.hh"
#include "PrettyPrint.hh"
#include "Error.hh"
#include "ClangCompat.hh"
#include "LLVMMisc.hh"
#include "IntervalTree.hh"

#include <unordered_set>
#include <iostream>

#include "clang/Rewrite/Core/Rewriter.h"

/* Ban symbols that we are sure to cause problems.  */

/* Although SourceManager has "translateFile" method, it seems unreliable
   because it may translate to the FileID that do not contain any changed
   buffer.  Hence we do our own thing here, which is look at our own
   FileEntry => FileID that we are sure to have modifications.  */
#pragma GCC poison translateFile

/****** Begin hack: used to get a pointer to a private member of a class. *****/
struct ASTUnit_TopLevelDecls
{
  typedef std::vector<Decl*> ASTUnit::*type;
  friend type Get(ASTUnit_TopLevelDecls);
};

template<typename Tag, typename Tag::type M>
struct Rob {
  friend typename Tag::type Get(Tag) {
    return M;
  }
};

template struct Rob<ASTUnit_TopLevelDecls, &ASTUnit::TopLevelDecls>;

/** Get pointer to the TopLevelDecls vector in the ASTUnit.
 *
 * The TopLevelDecls attribute from the AST is private, but we need to
 * access that in order to remove nodes from the AST toplevel vector,
 * else we can't remove further declarations of the function we need
 * to externalize.
 */
static std::vector<Decl *>* Get_Pointer_To_Toplev(ASTUnit *obj)
{
  return &(obj->*Get(ASTUnit_TopLevelDecls()));
}
/****************************** End hack.  ***********************************/

#define EXTERNALIZED_PREFIX "klpe_"
#define RENAME_PREFIX       "klpp_"

using namespace clang;
using namespace llvm;

/* IntervalTree.  */
using namespace Intervals;

// For debugging purposes.
#if 0
extern "C" void Debug_Range(const SourceRange &range)
{
  llvm::outs() << PrettyPrint::Get_Source_Text_Raw(range) << '\n';
}

#define DEBUG_RANGE(x) do { \
  llvm::outs() << "line " << __LINE__ << "  "; \
  Debug_Range(x); \
  } while (0)
#endif

/** Define a Visitor just to update TypeOfType instances:
 *
 *  Kernel code sometime does things like this:
 *
 *   typeof(symbol_to_externalize) x;
 *
 * this bizarre constructs come from macros, which sometimes explodes because
 * clang-extract is unable to determine which part of it generates the Decl
 * in question.
 *
 * Now we have to update those typeofs, but there is no easy way of parsing Types
 * to get to the Expr.  But the RecursiveASTVisitor knows how to do it, so we
 * hack the class in order to setup a call to
 * FunctionUpdate::Update_References_To_Symbol and update those references.
 *
 */
class TypeUpdaterVisitor : public RecursiveASTVisitor<TypeUpdaterVisitor>
{
  public:
    /* Constructor. Should match the FunctionUpdate constructor so we can
       instantiate it in the Visitor.  */
    TypeUpdaterVisitor(SymbolExternalizer &se, ValueDecl *new_decl,
                       const std::string &old_decl_name, bool wrap)
    : SE(se),
      NewSymbolDecl(new_decl),
      OldSymbolName(old_decl_name),
      Wrap(wrap)
    {}

  enum {
    VISITOR_CONTINUE = true,   // Return this for the AST transversal to continue;
    VISITOR_STOP     = false,  // Return this for the AST tranversal to stop completely;
  };

  /* The updator method.  This will be called by the Visitor when traversing the
     code.  */
  bool VisitTypeOfExprType(TypeOfExprType *type)
  {
    /* Create a instance of our tested-in-battle FunctionUpdater...  */
    SymbolExternalizer::FunctionUpdater fu(SE, NewSymbolDecl, OldSymbolName, Wrap);

    /* ... and call the method which updates the body of a function. (but that
       is not a function!  But who cares, and Expr is a special type of Stmt
       in clang so everything works!  */
    fu.Update_References_To_Symbol(type->getUnderlyingExpr());

    return VISITOR_CONTINUE;
  }

  /* To handle symbol renames on situations like the sizeof below
   *
   * static char x[4];
   *
   * void f(void) {
   *    char y[sizeof(x)];
   * }
   */
  bool VisitArrayTypeLoc(const ArrayTypeLoc &type)
  {
    SymbolExternalizer::FunctionUpdater fu(SE, NewSymbolDecl, OldSymbolName, Wrap);
    fu.Update_References_To_Symbol(type.getSizeExpr());

    return VISITOR_CONTINUE;
  }

  private:

  /** A reference to SymbolExternalizer.  */
  SymbolExternalizer &SE;

  /** The new variable declaration to replace the to be externalized function.  */
  ValueDecl *NewSymbolDecl;

  /** Name of the to be replaced function.  */
  const std::string &OldSymbolName;

  bool Wrap;
};

static std::vector<SourceRange>
Get_Range_Of_Identifier_In_SrcRange(const SourceRange &range, const char *id)
{
  std::vector<SourceRange> ret = {};
  StringRef string = PrettyPrint::Get_Source_Text(range);

  /* Tokenize away the function-like macro stuff or expression, we only want
     the identifier.  */
  const char *token_vector = " ().,;+-*/^|&{}[]<>^&|\r\n\t";

  /* Create temporary buff, strtok modifies it.  */
  unsigned len = string.size();
  char buf[len + 1];
  memcpy(buf, string.data(), len);
  buf[len] = '\0';

  char *tok = strtok(buf, token_vector);
  while (tok != nullptr) {
    if (strcmp(tok, id) == 0) {
      /* Found.  */
      ptrdiff_t distance = (ptrdiff_t)(tok - buf);
      assert(distance >= 0);

      /* Compute the distance from the original SourceRange of the
         MacroExpansion.  */
      int32_t offset = (int32_t) distance;
      SourceLocation start = range.getBegin().getLocWithOffset(offset);
      SourceLocation end = start.getLocWithOffset(strlen(tok)-1);

      /* Add to the list of output.  */
      ret.push_back(SourceRange(start, end));
    }

    tok = strtok(nullptr, token_vector);
  }

  return ret;
}

static std::vector<SourceRange>
Get_Range_Of_Identifier_In_SrcRange(const SourceRange &range, const StringRef id)
{
  return Get_Range_Of_Identifier_In_SrcRange(range, id.str().c_str());
}

static SourceRange Get_Range_For_Rewriter(const ASTUnit *ast, const SourceRange &range)
{
  const SourceManager &sm = ast->getSourceManager();

  /* Get a more precise source range of declaration.  */
  SourceLocation start = range.getBegin();

  /* Some declarations start with macro expansion, which the Rewriter
     class simple rejects.  Get one which it will accept.  */
  if (!start.isFileID()) {
    start = sm.getExpansionLoc(start);
  }

  SourceLocation end = Lexer::getLocForEndOfToken(
      range.getEnd(),
      0,
      sm,
      ast->getLangOpts());

  SourceRange new_range(start, end);
  return new_range;
}

bool SymbolExternalizer::Drop_Static(FunctionDecl *decl)
{
  if (decl->isStatic()) {
    auto ids = Get_Range_Of_Identifier_In_SrcRange(decl->getSourceRange(), "static");
    assert(ids.size() > 0 && "static decl without static keyword?");

    SourceRange static_range = ids[0];
    Remove_Text(static_range, 10);

    /* Update the storage class.  */
    decl->setStorageClass(StorageClass::SC_None);

    return true;
  }

  return false;
}

/* ---- Delta and TextModifications class ------ */


TextModifications::Delta::Delta(const SourceRange &to_change,
                                const std::string &new_text, int prio)
      : ToChange(to_change),
        NewText(new_text),
        Priority(prio)
{
  static int curr_id = 0;
  ID = curr_id++;
}

void TextModifications::Sort(void)
{
  auto comparator = [](const Delta& a, const Delta &b) {
    return a.Priority > b.Priority;
  };

  /* Sort by descending priority (larger comes first).  */
  std::sort(DeltaList.begin(), DeltaList.end(), comparator);
}

void TextModifications::Insert(Delta &delta)
{
  /* Insert into the vector.  */
  DeltaList.push_back(delta);
}

void TextModifications::Solve(void)
{
  /* Interval Tree to compute intersections of changes.  We can't have
     intersections so if we find those we try to solve them.  */
  IntervalTree<SourceLocation, const Delta&> interval_tree;
  std::vector<Delta> new_arr;

  /* Sort so that the highest priorities comes first.  */
  Sort();

  /* Get how many Text Modifications we desire.  */
  int n = DeltaList.size();

  /* Given one Text Modification, we must check if there is another text
     modification that intersects with this one.  If there is, we must
     remove the one with smaller priority (the one that is later on the
     vector because we sorted it).

     Use an Interval Tree to reduce the complexity from O(n(n-1)/2) to
     O(n log n).  */
  for (int i = 0; i < n; i++) {
    const Delta &a = DeltaList[i];
    auto overlap = interval_tree.findOverlappingIntervals(a.ToChange);
    /* Check if we already have this range in the tree.  */
    if (overlap.size() > 0) {
      const Delta &b = overlap[0].value;

      if (a.Priority <= b.Priority || a == b) {
        // Ignore.
      } else {
        /* Intersection with the same priority. Issue an error -- we can't have this.  */
        DiagsClass::Emit_Error("Rewriter ranges with same priority intersects");
        DiagsClass::Emit_Note(" This one: (priority " + std::to_string(a.Priority) + ')',
                              a.ToChange);
        DiagsClass::Emit_Note("with this: (priority " + std::to_string(b.Priority) + ')',
                              b.ToChange);
        throw std::runtime_error("SymbolExternalizer can not continue.");
      }
    } else {
      interval_tree.insert(Interval<SourceLocation, const Delta&>(a.ToChange, a));
      new_arr.push_back(a);
    }
  }
  DeltaList = new_arr;
}

bool TextModifications::Insert_Into_FileEntryMap(const SourceLocation &loc)
{
  /* Register the changed FileID for retrieving the buffer later.  */
  FileID begin_id = SM.getFileID(loc);

  /* Insert into the list of FileIDs.  */
  const FileEntry *fentry = SM.getFileEntryForID(begin_id);

  /* There are some cases where the fentry is known to return NULL.  Check if
     those are the cases we already acknownledged.  */
  if (fentry == nullptr) {
    PresumedLoc ploc = SM.getPresumedLoc(loc);
    if (ploc.getFilename() == StringRef("<command line>")) {
      /* Locations comming from the command line can be ignored.  */
      return false;
    }

    /* Crash with assertion.  */
    assert(fentry && "FileEntry is NULL on a non-acknowledged case");
  }

  /* Insert the FileEntry if we don't have one.  */
  if (FileEntryMap.find(fentry) == FileEntryMap.end()) {
    /* Insert it.  */
    FileEntryMap[fentry] = begin_id;
    return true;
  }

  /* Nothing was inserted.  */
  return false;
}

bool TextModifications::Insert_Into_FileEntryMap(const SourceRange &range)
{
    /* Register the changed FileID for retrieving the buffer later.  */
    FileID begin_id = SM.getFileID(range.getBegin());
    FileID end_id   = SM.getFileID(range.getEnd());

    /* Ensure that the fileIDs are equal.  */
    assert(begin_id == end_id);

    return Insert_Into_FileEntryMap(range.getBegin());
}

void TextModifications::Commit(void)
{
  Solve();
  int n = DeltaList.size();

  for (int i = 0; i < n; i++) {
    // Commit Change.
    Delta &a = DeltaList[i];

    /* Try to insert into the FileEntryMap for commiting the change to the buffer
       later.  */
    Insert_Into_FileEntryMap(a.ToChange);

    /* Get the text we want to change.  We only do this to know its length.  */
    StringRef source_text = Lexer::getSourceText(CharSourceRange::getCharRange(
                                                 a.ToChange),
                                                 SM, LO);
    unsigned len = source_text.size();

    /* ReplaceText(SourceRange, StringRef) version is unreliable in llvm-16.  */
    assert(RW.ReplaceText(a.ToChange.getBegin(), len, a.NewText) == false);

    if (DumpingEnabled) {
      Dump(i, a);
    }
  }
}

void TextModifications::Dump(unsigned num, const Delta &a)
{
  std::string output_file = "/tmp/Externalizer.dump." + std::to_string(num) + ".c";
  FILE *file = fopen(output_file.c_str(), "w");

  llvm::outs() << "Generating " + output_file + '\n';

  std::string note = std::to_string(num) + " Changing " +
                PrettyPrint::Get_Source_Text_Raw(a.ToChange).str() +
                " to " + a.NewText;
  note = "/*\n" + note + "*/\n";
  fputs(note.c_str(), file);

  /* Iterate into all files we changed.  */
  for (auto it = FileEntryMap.begin(); it != FileEntryMap.end(); ++it) {
    /*
    const FileEntry *fentry = it->first;
    */
    FileID id = it->second;

    /* Our updated file buffer.  */
    const RewriteBuffer *rewritebuf = RW.getRewriteBufferFor(id);

    /* If we have modifications, then dump the buffer.  */
    if (rewritebuf) {
      /* The RewriteBuffer object contains a sequence of deltas of the original
         buffer.  Clearly a faster way would be apply those deltas into the
         target buffer directly, but clang doesn't seems to provide such
         interface, so we expand them into a temporary string and them pass
         it to the SourceManager.  */
      std::string modified_str = std::string(rewritebuf->begin(), rewritebuf->end());
      fputs(modified_str.c_str(), file);
      fputs("\n\n/* --------- */\n\n", file);
    }
  }

  fclose(file);
}

bool TextModifications::Is_Before_Or_Equal(const PresumedLoc &a, const PresumedLoc &b)
{
  unsigned a_line = a.getLine();
  unsigned a_col  = a.getColumn();
  unsigned b_line = b.getLine();
  unsigned b_col  = b.getColumn();

  unsigned m = std::max(a_col, b_col);
  unsigned psi_a = m*a_line + a_col;
  unsigned psi_b = m*b_line + b_col;

  return psi_a <= psi_b;
}

bool TextModifications::Intersects(const SourceRange &a, const SourceRange &b)
{
  PresumedLoc a_begin = SM.getPresumedLoc(a.getBegin());
  PresumedLoc a_end   = SM.getPresumedLoc(a.getEnd());
  PresumedLoc b_begin = SM.getPresumedLoc(b.getBegin());
  PresumedLoc b_end   = SM.getPresumedLoc(b.getEnd());

  assert(a_begin.getFileID() == a_end.getFileID());
  assert(b_begin.getFileID() == b_end.getFileID());

  if (a_begin.getFileID() != b_begin.getFileID()) {
    /* Files are distinct, thus we can't easily determine which comes first.  */
    return false;
  }

  assert(a_begin.getLine() <= a_end.getLine());
  assert(b_begin.getLine() <= b_end.getLine());

  return Is_Before_Or_Equal(a_begin, b_end) && Is_Before_Or_Equal(b_begin, a_end);
}

bool TextModifications::Intersects(const Delta &a, const Delta &b)
{
  return Intersects(a.ToChange, b.ToChange);
}

/* ---- End of Deltas class -------- */

bool SymbolExternalizer::FunctionUpdater::Update_References_To_Symbol(Stmt *stmt)
{
  if (!stmt)
    return false;

  bool replaced = false;

  if (DeclRefExpr::classof(stmt)) {
    DeclRefExpr *expr = (DeclRefExpr *) stmt;
    ValueDecl *decl = expr->getDecl();

    /* In case we modified the Identifier of the original function, getName()
       will return the name of the new function but the SourceText will not
       be updated.  Hence check if the SourceRange has it as well.  */
    auto vec_of_ranges = Get_Range_Of_Identifier_In_SrcRange(expr->getSourceRange(),
                                                             OldSymbolName.c_str());
    StringRef old_name_src_txt = "";
    if (!vec_of_ranges.empty()) {
      old_name_src_txt = PrettyPrint::Get_Source_Text(vec_of_ranges[0]);
    }

    if (decl->getName() == OldSymbolName || old_name_src_txt == OldSymbolName) {
      /* Rewrite the source code.  */
      SourceLocation begin = expr->getBeginLoc();
      SourceLocation end = expr->getEndLoc();

      SourceRange range(begin, end);
      StringRef str = PrettyPrint::Get_Source_Text(range);

      /* Ensure that we indeed got the old symbol.  */
      if (str == OldSymbolName) {
        /* Prepare the text modification.  */
        std::string new_name;
        if (Wrap) {
          new_name = NewSymbolDecl->getName().str();
        } else {
          new_name = "(*" + NewSymbolDecl->getName().str() + ")";
        }

        /* Issue a text modification.  */
        SE.Replace_Text(range, new_name, 100);
      } else {
        /* If we did not get the old symbol, it mostly means that the
           references comes from a macro.  */

        //std::cout << "WARNING: Unable to find location of symbol name: " << OldSymbolName << '\n';
      }

      /* Replace reference with the rewiten name.  */
      expr->setDecl(NewSymbolDecl);
      replaced = true;
    }
  }

  /* Repeat the process to child statements.  */
  clang::Stmt::child_iterator it, it_end;
  for (it = stmt->child_begin(), it_end = stmt->child_end();
      it != it_end; ++it) {

    Stmt *child = *it;
    replaced |= Update_References_To_Symbol(child);
  }

  return replaced;
}

bool SymbolExternalizer::FunctionUpdater::Update_References_To_Symbol(DeclaratorDecl *to_update)
{
  ToUpdate = to_update;
  if (to_update) {
    if (VarDecl *vdecl = dyn_cast<VarDecl>(to_update)) {
      return Update_References_To_Symbol(vdecl->getInit());
    }
    if (FunctionDecl *fdecl = dyn_cast<FunctionDecl>(to_update)) {
      return Update_References_To_Symbol(fdecl->getBody());
    }
  }
  return false;
}

void SymbolExternalizer::Replace_Text(const SourceRange &range, StringRef new_name, int prio)
{
  SourceRange rw_range = Get_Range_For_Rewriter(AST, range);
  TextModifications::Delta delta(rw_range, new_name.str(), prio);
  TM.Insert(delta);
}

void SymbolExternalizer::Remove_Text(const SourceRange &range, int prio)
{
  Replace_Text(range, "", prio);
}

void SymbolExternalizer::Insert_Text(const SourceLocation &loc, StringRef text)
{
  TM.Insert_Into_FileEntryMap(loc);
  TM.Get_Rewriter().InsertText(loc, text);
}

VarDecl *SymbolExternalizer::Create_Externalized_Var(DeclaratorDecl *decl, const std::string &name)
{
  /* Hack a new Variable Declaration node in which holds the address of our
     externalized symbol.  We use the information of the old function to
     build it. For example, assume we want to externalize:

     int f(int);

     then we need to emit:

     static int (*f)(int);

     instead of the first function delcaration.  Therefore we need:
      - The new node's type must be a pointer to the original function type.
      - The source location of the new node should somewhat match the original
        symbol.
      - We should append a `static` to it so it doesn't clash with other
        livepatches.  */

  /* Create a new identifier with name `name`.  */
  IdentifierInfo *id = AST->getPreprocessor().getIdentifierInfo(name);

  /* Get clang's ASTContext, needed to allocate the node.  */
  ASTContext &astctx = AST->getASTContext();

  /* Create a type that is a pointer to the externalized object's type.  */
  QualType pointer_to = astctx.getPointerType(decl->getType());

  /* For IBT, create an extern variable with the same type from the original */
  if (Ibt)
    pointer_to = decl->getType();

  /* Get context of decl.  */
  DeclContext *decl_ctx;

  /* Create node.  */
  if (FunctionDecl *fdecl = dyn_cast<FunctionDecl>(decl)) {
    decl_ctx = fdecl->getParent();
  } else {
    decl_ctx = decl->getDeclContext();
  }

  /*
   * For normal externalization, create a static variable. When dealing with IBT
   * restrictions, create an extern variable that will be sorted out later by
   * the code that is using it, like Linux for example.
   */
  StorageClass sc = SC_Static;
  if (Ibt)
    sc = SC_Extern;

  VarDecl *ret = VarDecl::Create(astctx, decl_ctx,
    decl->getBeginLoc(),
    decl->getEndLoc(),
    id,
    pointer_to,
    nullptr,
    sc
  );

  /* return node.  */
  return ret;
}

bool SymbolExternalizer::Commit_Changes_To_Source(
                          IntrusiveRefCntPtr<llvm::vfs::OverlayFileSystem> &ofs,
                          IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> &mfs,
                          std::vector<std::string> &includes_to_expand)
{
  SourceManager &sm = AST->getSourceManager();
  bool modified = false;
  bool main_file_inserted = false;

  auto new_ofs = IntrusiveRefCntPtr<vfs::OverlayFileSystem>(
                       new vfs::OverlayFileSystem(vfs::getRealFileSystem()));

  auto new_mfs = IntrusiveRefCntPtr<vfs::InMemoryFileSystem>(
                        new vfs::InMemoryFileSystem);

  new_ofs->pushOverlay(new_mfs);

  /* Commit the text changes.  */
  TM.Commit();

  Rewriter &RW = TM.Get_Rewriter();
  auto FileEntryMap = TM.Get_FileEntry_Map();

  /* Iterate into all files we may have opened, most probably headers that are
     #include'd.  */

  for (auto it = FileEntryMap.begin(); it != FileEntryMap.end(); ++it) {
    /* Although SourceManager has "translateFile" method, it seems unreliable
       because it may translate to the FileID that do not contain any changed
       buffer.  Hence we do our own thing here, which is look at our own
       FileEntry => FileID that we are sure to have modifications.  */
    const FileEntry *fentry = it->first;
    FileID id = it->second;

    const RewriteBuffer *rewritebuf = RW.getRewriteBufferFor(id);

    /* If we have modifications, then update the buffer.  */
    if (rewritebuf) {
      /* The RewriteBuffer object contains a sequence of deltas of the original
         buffer.  Clearly a faster way would be apply those deltas into the
         target buffer directly, but clang doesn't seems to provide such
         interface, so we expand them into a temporary string and them pass
         it to the SourceManager.  */
      std::string modified_str = std::string(rewritebuf->begin(), rewritebuf->end());

      if (new_mfs->addFile(fentry->getName(),
                       0, MemoryBuffer::getMemBufferCopy(modified_str)) == false) {
        llvm::outs() << "Unable to add " << fentry->getName() << " into InMemoryFS.\n";
      }

      /* In case this is not the main file, we need to mark it for expansion.  */
      if (id != sm.getMainFileID()) {
        StringRef file_name = fentry->getName();
        includes_to_expand.push_back(file_name.str());
      } else {
        main_file_inserted = true;
      }

      modified = true;
    }
  }

  /* Make sure we inserted the main file.  */
  if (main_file_inserted == false) {
      FileID id = sm.getMainFileID();
      const FileEntry *fentry = sm.getFileEntryForID(id);
      const std::string &main_file_content = Get_Modifications_To_Main_File();
      if (new_mfs->addFile(fentry->getName(),
                       0,
                       MemoryBuffer::getMemBufferCopy(main_file_content)) == false) {
        llvm::outs() << "Unable to add " << fentry->getName() << " into InMemoryFS.\n";
      }
  }

  ofs = new_ofs;
  mfs = new_mfs;

  return modified;
}

std::string SymbolExternalizer::Get_Modifications_To_Main_File(void)
{

  SourceManager &sm = AST->getSourceManager();

  /* Our updated file buffer for main file.  */
  FileID main_id = sm.getMainFileID();
  Rewriter &RW = TM.Get_Rewriter();
  RewriteBuffer &main_buf = RW.getEditBuffer(main_id);

  return std::string(main_buf.begin(), main_buf.end());
}

void SymbolExternalizer::Strongly_Externalize_Symbol(const std::string &to_externalize)
{
  _Externalize_Symbol(to_externalize, ExternalizationType::STRONG);
}

void SymbolExternalizer::Weakly_Externalize_Symbol(const std::string &to_externalize)
{
  _Externalize_Symbol(to_externalize, ExternalizationType::WEAK);
}

/** Given a MacroExpansion object, we try to get the location of where the token
    appears on it.

    TODO: clang may provide a way of doing this with a tokenizer, so maybe this
    code can become cleaner with it.  */
static std::vector<SourceRange>
Get_Range_Of_Identifier_In_Macro_Expansion(const MacroExpansion *exp, const char *id)
{
  return Get_Range_Of_Identifier_In_SrcRange(exp->getSourceRange(), id);
}

void SymbolExternalizer::Rewrite_Macros(std::string const &to_look_for, std::string const &replace_with)
{
  PreprocessingRecord *rec = AST->getPreprocessor().getPreprocessingRecord();

  for (PreprocessedEntity *entity : *rec) {
    if (MacroDefinitionRecord *def = dyn_cast<MacroDefinitionRecord>(entity)) {
      MacroInfo *info = MW.Get_Macro_Info(def);

      if (!info)
        continue;

      for (const Token tok : info->tokens()) {
        IdentifierInfo *id_info = tok.getIdentifierInfo();
        if (!id_info)
          continue;

        MacroInfo *maybe_macro = MW.Get_Macro_Info(id_info, def->getLocation());

        if (!maybe_macro && !MacroWalker::Is_Identifier_Macro_Argument(info, id_info)) {
          if (id_info->getName() == to_look_for) {
            Replace_Text(SourceRange(tok.getLocation(), tok.getLastLoc()), replace_with, 10);
          }
        }
      }
    } else if (MacroExpansion *exp = dyn_cast<MacroExpansion>(entity)) {
      /* We must look for references to externalized variables in funcion-like
          macro expansions on the program's toplevel.  */
      auto ranges = Get_Range_Of_Identifier_In_Macro_Expansion(exp, to_look_for.c_str());

      for (SourceRange &tok_range : ranges) {
        Replace_Text(tok_range, replace_with, 10);
      }
    }
  }
}

bool SymbolExternalizer::_Externalize_Symbol(const std::string &to_externalize,
                                             ExternalizationType type)
{
  ASTUnit::top_level_iterator it;
  bool first = true;
  ValueDecl *new_decl = nullptr;
  bool must_update = false;
  bool wrap = false;
  bool externalized = false;

  /* The TopLevelDecls attribute from the AST is private, but we need to
     access that in order to remove nodes from the AST toplevel vector,
     else we can't remove further declarations of the function we need
     to externalize.  */
  std::vector<Decl *> *topleveldecls = Get_Pointer_To_Toplev(AST);

  for (it = AST->top_level_begin(); it != AST->top_level_end(); ++it) {
    DeclaratorDecl *decl = dynamic_cast<DeclaratorDecl *>(*it);

    /* If we externalized some function, then we must start analyzing for further
       functions in order to find if there is a reference to the function we
       externalized.  */
    if (must_update) {
      /* Call our hack to update the TypeOfTypes.  */
      TypeUpdaterVisitor(*this, new_decl, to_externalize, wrap)
        .TraverseDecl(decl);

      FunctionUpdater(*this, new_decl, to_externalize, wrap || Ibt)
        .Update_References_To_Symbol(decl);
    }

    if (decl && decl->getName() == to_externalize) {
      if (type == ExternalizationType::STRONG) {
        if (first) {
          /* If we found the first instance of the function we want to externalize,
             then proceed to create and replace the function declaration node with
             a variable declaration node of proper type.  */
          std::string old_name = decl->getName().str();
          std::string new_name = EXTERNALIZED_PREFIX + old_name;
          new_decl = Create_Externalized_Var(decl, new_name);
          Log.push_back({.OldName = old_name,
                         .NewName = new_name,
                         .Type = ExternalizationType::STRONG});

          /* Create a string with the new variable type and name.  */
          std::string o;
          llvm::raw_string_ostream outstr(o);

          /*
           * It won't be a problem to add the code below multiple times, since
           * clang-extract will remove ifndefs for already defined macros
           */
          if (Ibt) {
            outstr << "#ifndef KLP_RELOC_SYMBOL_POS\n"
                      "# define KLP_RELOC_SYMBOL_POS(LP_OBJ_NAME, SYM_OBJ_NAME, SYM_NAME, SYM_POS) \\\n"
                      "   asm(\"\\\".klp.sym.rela.\" #LP_OBJ_NAME \".\" #SYM_OBJ_NAME \".\" #SYM_NAME \",\" #SYM_POS \"\\\"\")\n"
                      "# define KLP_RELOC_SYMBOL(LP_OBJ_NAME, SYM_OBJ_NAME, SYM_NAME) \\\n"
                      "   KLP_RELOC_SYMBOL_POS(LP_OBJ_NAME, SYM_OBJ_NAME, SYM_NAME, 0)\n"
                      "#endif\n\n";
          }

          new_decl->print(outstr, AST->getLangOpts());

          if (Ibt) {
            std::string sym_mod = IA.Get_Symbol_Module(old_name);
            if (sym_mod == "")
              sym_mod = "vmlinux";

            outstr << " \\\n" << "\tKLP_RELOC_SYMBOL(" << PatchObject << ", " <<
                   sym_mod << ", " << old_name << ")";
          }
          outstr << ";\n";

          Replace_Text(decl->getSourceRange(), outstr.str(), 1000);

          must_update = true;
          wrap = false;

          std::string replacement = "(*" + new_decl->getName().str() + ")";
          /*
           * IBT uses extern variables, so we need to use the same type from the
           * private symbol.
           */
          if (Ibt)
            replacement = new_decl->getName().str();

          /* Update any macros that may reference the symbol.  */
          Rewrite_Macros(to_externalize, replacement);

          /* Slaps the new node into the position of where was the function
             to be externalized.  */
          *it = new_decl;
          first = false;
          externalized = true;

        } else {
          /* If we externalized this function, then all further delcarations of
             this function shall be discarded.  */

          /* Get source location of old function declaration.  */
          Remove_Text(decl->getSourceRange(), 1000);

          /* Remove node from AST.  */
          topleveldecls->erase(it);
          /* We must decrease the iterator because we deleted an element from the
             vector.  */
          it--;
        }
      } else if (type == ExternalizationType::WEAK) {
        /* Now checks if this is a function or a variable delcaration.  */
        if (FunctionDecl *func = dyn_cast<FunctionDecl>(decl)) {
          /* In the case it is a function we need to remove its declaration that
             have a body.  */
          if (func->hasBody()) {
            FunctionDecl *with_body = func->getDefinition();
            externalized = true;
            if (with_body == func) {
              /* Damn. This function do not have a prototype, we will have to
                 craft it ourself.  */

              /* FIXME: This reults in unwanted intersections.  */
#if 0
              Stmt *body = with_body->getBody();
              Replace_Text(body->getSourceRange(), ";\n", 1000);

              /* Remove the body from the AST.  */
              with_body->setBody(nullptr);
#endif
            } else {
              Remove_Text(with_body->getSourceRange(), 1000);
              topleveldecls->erase(it);

              /* We must decrease the iterator because we deleted an element from the
                 vector.  */
              it--;
            }
          }
        }
      } else if (type == ExternalizationType::RENAME) {
        /* Get SourceRange where the function identifier is.  */
        auto ids = Get_Range_Of_Identifier_In_SrcRange(decl->getSourceRange(),
                                                       decl->getName());
        assert(ids.size() > 0 && "Decl name do not match required identifier?");

        SourceRange id_range = ids[0];
        std::string new_name = RENAME_PREFIX + decl->getName().str();
        if (first) {
          /* Only register the first decl rename of the same variable.  */
          Log.push_back({.OldName = decl->getName().str(),
                         .NewName = new_name,
                         .Type = ExternalizationType::RENAME});
          first = false;
        }

        /* In the case there is a `static` modifier in function, try to drop it.  */
        if (FunctionDecl *fdecl = dyn_cast<FunctionDecl>(decl)) {
          Drop_Static(fdecl);
        }

        /* Rename the declaration.  */
        IdentifierInfo *new_id = AST->getPreprocessor().getIdentifierInfo(new_name);
        DeclarationName new_decl_name(new_id);

        decl->setDeclName(new_decl_name);
        new_decl = decl;

        /* Replace text content of old declaration.  */
        Replace_Text(id_range, new_name, 100);

        must_update = true;
        wrap = true;

        /* Update any macros that may reference the symbol.  */
        Rewrite_Macros(to_externalize, new_name);
        externalized = true;
      }
    }
  }

  return externalized;
}

void SymbolExternalizer::Externalize_Symbol(const std::string &to_externalize)
{
  /* If the symbol is available in the debuginfo and is an EXTERN symbol, we
     do not need to rewrite it, but rather we need to erase any declaration
     with body of it.  */
  if (IA.Can_Decide_Visibility()) {
    if (IA.Is_Externally_Visible(to_externalize)) {
      Weakly_Externalize_Symbol(to_externalize);
    } else {
      Strongly_Externalize_Symbol(to_externalize);
    }
  } else {
    /* Well, we don't have information so we simply strongly externalize
       everything.  */
    Strongly_Externalize_Symbol(to_externalize);
  }
}

void SymbolExternalizer::Externalize_Symbols(std::vector<std::string> const &to_externalize_array)
{
  for (const std::string &to_externalize : to_externalize_array) {
    Externalize_Symbol(to_externalize);
  }
}

void SymbolExternalizer::Rename_Symbols(std::vector<std::string> &to_rename_array)
{
  for (std::string &to_externalize : to_rename_array) {
    if (_Externalize_Symbol(to_externalize, ExternalizationType::RENAME)) {
      /* Update the function names for the ClosurePass.  */
      to_externalize = RENAME_PREFIX + to_externalize;
    }
  }
}
