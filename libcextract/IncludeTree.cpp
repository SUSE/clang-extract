//===- IncludeTree.cpp - Tree of #include'd files -------------- *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Tree of #include'd files.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#include "IncludeTree.hh"
#include "PrettyPrint.hh"
#include "ClangCompat.hh"
#include "Error.hh"

#include <stack>
#include <string>
#include <algorithm>

static bool In_Set(std::unordered_set<std::string> &set, const std::string &str,
                   bool remove_if_exists)
{
  if (set.find(str) != set.end()) {
    if (remove_if_exists) {
      set.erase(str);
    }
    return true;
  }

  return false;
}

/* ----- IncludeTree ------ */
IncludeTree::IncludeTree(Preprocessor &pp,
                         SourceManager &sm,
                         IncludeExpansionPolicy::Policy p,
                         std::vector<std::string> const &must_expand)
  : PP(pp),
    SM(sm),
    IEP(IncludeExpansionPolicy::Get_Expansion_Policy_Unique(p))
{
  Build_Header_Tree(must_expand);
  Fix_Output_Attrs(Root);
  Build_Header_Map();
}

IncludeTree::~IncludeTree(void)
{
  delete Root;
}

void IncludeTree::Build_Header_Tree(std::vector<std::string> const &must_expand)
{
  /* Construct the IncludeTree. The algorithm works as follows:

       Push the compiling file to the top of the stack.
    1. Analyze the PreprocessingRecord in sequence for includes.
    2. We wait for the first include comming from the main file. In
       case there are -include commands in compilation, the origin file
       will come as invalid and we need to be careful with this later.
    3. Once we find an include, we check to see in which file this was
       written, and rewind the stack until we find this file. This is
       because the includes are parsed in a DFS fashion, hence the stack.
    4. Add the found include as a child and update its parent.  */

  std::unordered_set<std::string> must_expand_set(must_expand.begin(),
                                                  must_expand.end());
  MacroWalker mw(PP);
  bool already_seen_main = false;
  SourceManager *SM = PrettyPrint::Get_Source_Manager();
  OptionalFileEntryRef main = SM->getFileEntryRefForID(SM->getMainFileID());

  std::stack<IncludeNode *> stack;
  Root = new IncludeNode(this);

  PreprocessingRecord *rec = PP.getPreprocessingRecord();
  for (PreprocessedEntity *entity : *rec) {
    if (InclusionDirective *id = dyn_cast<InclusionDirective>(entity)) {

      /* In which file this include was written?  */
      SourceRange range = id->getSourceRange();
      OptionalFileEntryRef this_file = PrettyPrint::Get_FileEntry(range.getBegin());

      /* This is very confusing, but that is how clang handle things.
         -include HEADER can be passed as a command line argument to clang. in
         this case, the header do not come from a valid source location and
         hence the `this_file` object will come without a value. Hence every
         time we find such case we must discard the stack because we are
         processing a new file.

         However, when we find the main file this kind of analysis is not
         needed anymore. But now we must have a way to check if the node on top
         of the stack is the main file.  Hence we create a node representing
         the main file.  */

      if (!already_seen_main) {
        if (!this_file.has_value()) {
          // Discard stack and push root.
          stack = std::stack<IncludeNode *>();
          stack.push(Root);
        } else if (this_file.has_value() && *this_file == *main) {
          // Set the root FileEntry to the main file now.
          Root->Set_FileEntry(main);

          stack = std::stack<IncludeNode *>();
          stack.push(Root);
          already_seen_main = true;
        }
      }

      /* Pop the stack until we find which HeaderNode we currently are.  */
      IncludeNode *current = stack.top();
      while (!current->Is_Same_File(this_file)) {
        stack.pop();
        current = stack.top();
      }

      bool expand = In_Set(must_expand_set, id->getFileName().str(), /*remove=*/true) ||
                    In_Set(must_expand_set, id->getFile()->getName().str(),
                           /*remove=*/true);

      /* Starting from LLVM-18 and in some platforms (s390x), the behaviour of
       * FileEntry::getName() changed. Now it returns the full path of the file
       * rather than the relative path to it, so here we account it for now onwards.
       */
      expand |= In_Set(must_expand_set,
                       id->getFile()->getFileEntry().tryGetRealPathName().str(),
                       /*remove=*/true);

      bool output = already_seen_main && current->Should_Be_Expanded()
                                      && !expand;
      bool is_from_minus_include = !already_seen_main;

      /* Add child to tree.  */
      IncludeNode *child = new IncludeNode(this,
                                           id,
                                           output,
                                           expand,
                                           is_from_minus_include);
      current->Add_Child(child);
      child->Set_Parent(current);

      /* In the case it was marked for expansion, then update parent nodes.  */
      if (expand)
        child->Mark_For_Expansion();

      stack.push(child);
    } else if (MacroDefinitionRecord *def = dyn_cast<MacroDefinitionRecord>(entity)) {
      MacroInfo *info = mw.Get_Macro_Info(def);
      if (info && info->isUsedForHeaderGuard()) {
        const SourceRange &range = def->getSourceRange();
        OptionalFileEntryRef this_file = PrettyPrint::Get_FileEntry(range.getBegin());

        /* Pop the stack until we find which HeaderNode we currently are.  */
        IncludeNode *current = stack.top();
        while (!current->Is_Same_File(this_file)) {
          stack.pop();
          current = stack.top();
        }
        current->Set_HeaderGuard(def);
      }
    }
  }

  /* In the case there is no includes, the Root node is never set to the main
     file.  Do it now.  */
  if (already_seen_main == false) {
    Root->Set_FileEntry(main);
  }

  /* Root node should always be set to expand and not output.  */
  assert(Root->Should_Be_Expanded() && !Root->Should_Be_Output());

  /* Check if the Root has a valid FileEntry.  */
  assert(Root->Get_FileEntry().has_value());
}

void IncludeTree::Build_Header_Map(void)
{
  static bool warned = false;
  std::stack<IncludeNode *> stack;

  stack.push(Root);
  while (!stack.empty()) {
    IncludeNode *node = stack.top();
    stack.pop();

    /* For Files.  */
    OptionalFileEntryRef file = node->Get_FileEntry();
    const FileEntryRef fentry = *file;
    if (Map.find(fentry) != Map.end()) {
      /* FIXME: Find a way to correcly map the FileEntry to the node instead of
         discarding future appearances.  */
      if (warned == false) {
        const SourceRange &range = node->Get_Include_Spelling_Range();
        DiagsClass::Emit_Warn("project #include's the same file multiple times."
                              " Only the first is registered.", range);
        warned = true;
      }
    } else {
      Map[fentry] = node;
    }

    /* For InclusionDirectives.  */
    if (node->ID != nullptr) {
      IncMap[node->ID] = node;
    }

    int n = node->Get_Num_Childs();
    while (n > 0) {
      stack.push(node->Get_Child(--n));
    }
  }
}

void IncludeTree::Fix_Output_Attrs(IncludeNode *node)
{
  if (node->ShouldBeOutput) {
    node->Mark_For_Output();
  }

  if (node->ShouldBeOutput || node->ShouldBeExpanded) {
    for (IncludeNode *child : node->Childs) {
      Fix_Output_Attrs(child);
    }
  }
}

IncludeNode *IncludeTree::Get(const SourceLocation &loc)
{
  OptionalFileEntryRef fileref = PrettyPrint::Get_FileEntry(loc);

  /* In case we could not find a FileRef, then try the ExpansionLoc.  */
  if (!fileref.has_value()) {
    SourceManager *SM = PrettyPrint::Get_Source_Manager();
    const SourceLocation &loc2 = SM->getExpansionLoc(loc);
    fileref = PrettyPrint::Get_FileEntry(loc2);
  }

  if (fileref.has_value()) {
    const FileEntry *file = &fileref->getFileEntry();
    return Map[file];
  }

  /* Well, the declaration seems fuzzy, so return that we don't have a file.  */
  return nullptr;
}

void IncludeTree::Dump(llvm::raw_ostream &out)
{
  Root->Dump(out);
}

void IncludeTree::Dump(void)
{
  Root->Dump();
}

/* ----- IncludeNode ------ */

IncludeTree::IncludeNode::IncludeNode(IncludeTree *tree,
                                      InclusionDirective *include,
                                      bool output, bool expand,
                                      bool is_from_minus_include)
  : Tree(*tree),
    ID(include),
    File(ID->getFile()),
    HeaderGuard(nullptr),
    ShouldBeOutput(output),
    ShouldBeExpanded(expand),
    IsFromMinusInclude(is_from_minus_include),
    Parent(nullptr)
{
}

IncludeTree::IncludeNode::IncludeNode(IncludeTree *tree)
  : Tree(*tree),
    ID(nullptr),
    HeaderGuard(nullptr),
    ShouldBeOutput(false),
    ShouldBeExpanded(true),
    IsFromMinusInclude(false),
    Parent(nullptr)
{
  memset(&File, 0, sizeof(File));
}

IncludeTree::IncludeNode::~IncludeNode(void)
{
  for (auto child : Childs) {
    delete child;
  }

  Childs.clear();
}

OptionalFileEntryRef IncludeTree::IncludeNode::Get_FileEntry(void)
{
  return File;
}

bool IncludeTree::IncludeNode::Is_Same_File(IncludeNode *node)
{
  return (this == node) || Is_Same_File(node->Get_FileEntry());
}

bool IncludeTree::IncludeNode::Is_Same_File(OptionalFileEntryRef file)
{

  /* It is possible that a header was included by no file, which can result
     in a null FileEntry.  Such cases are caused by the -include option during
     compilation.  In this case, we say that two null files are equal.  */

  if (!File.has_value()) {
    if (!file.has_value()) {
      return true;
    }
    return false;
  } else if (!file.has_value()) {
    return false;
  }

  return *File == *file;
}

void IncludeTree::IncludeNode::Add_Child(IncludeTree::IncludeNode *node)
{
  Childs.push_back(node);
}

void IncludeTree::IncludeNode::Set_Parent(IncludeTree::IncludeNode *node)
{
  Parent = node;
}

IncludeTree::IncludeNode *IncludeTree::IncludeNode::Get_Parent(void)
{
  return Parent;
}

std::vector<IncludeTree::IncludeNode *> *IncludeTree::IncludeNode::Get_Childs(void)
{
  return &Childs;
}

IncludeTree::IncludeNode *IncludeTree::IncludeNode::Get_Child(unsigned i)
{
  return Childs[i];
}

unsigned IncludeTree::IncludeNode::Get_Num_Childs(void)
{
  return Childs.size();
}

void IncludeTree::IncludeNode::Set_FileEntry(OptionalFileEntryRef file)
{
  File = file;
}

SourceRange IncludeTree::IncludeNode::Get_File_Range(void)
{
  SourceManager *SM = PrettyPrint::Get_Source_Manager();

  SourceLocation start, end;
  FileID fid = SM->getOrCreateFileID(*File, SrcMgr::CharacteristicKind());
  start = SM->getLocForStartOfFile(fid);
  end = SM->getLocForEndOfFile(fid);

  assert(start.isValid() && "Start of header is invalid.");
  assert(end.isValid() && "End of header is invalid.");

  return SourceRange(start, end);
}

SourceRange IncludeTree::IncludeNode::Get_Include_Spelling_Range(void)
{
  if (ID) {
    return ID->getSourceRange();
  }

  assert(false && "Attempt to take SourceRange of root node.");
}

StringRef IncludeTree::IncludeNode::Get_Filename(void)
{
  if (ID) {
    return ID->getFileName();
  }

  return StringRef("<Root node>");
}

std::unique_ptr<std::vector<IncludeNode *>> IncludeTree::Get_Non_Expand_Includes(void)
{
  auto vec = new std::vector<IncludeNode *>();
  std::stack<IncludeNode *> stack;

  stack.push(Root);
  while (!stack.empty()) {
    IncludeNode *node = stack.top();
    stack.pop();

    bool should_output = node->Should_Be_Output();
    bool should_expand = node->Should_Be_Expanded();

    assert(!(should_output && should_expand) &&
           "Include can can not be expanded and outputed at the same time!");

    if (!should_expand) {
      vec->push_back(node);
    } else {
      int n = node->Get_Num_Childs();
      while (n > 0) {
        stack.push(node->Get_Child(--n));
      }
    }
  }

  return std::unique_ptr<std::vector<IncludeNode *>>{vec};
}

std::unique_ptr<std::vector<IncludeNode *>> IncludeTree::Get_Includes(void)
{
  auto vec = new std::vector<IncludeNode *>();
  std::stack<IncludeNode *> stack;

  stack.push(Root);
  while (!stack.empty()) {
    IncludeNode *node = stack.top();
    stack.pop();

    /* Nodes without InclusionDirective are just for making the algorithm easier.  */
    if (node->Get_InclusionDirective() != nullptr) {
      vec->push_back(node);
    }
    int n = node->Get_Num_Childs();
    while (n > 0) {
      stack.push(node->Get_Child(--n));
    }
  }

  return std::unique_ptr<std::vector<IncludeNode *>>{vec};
}

IncludeNode *IncludeTree::Get(const InclusionDirective *directive)
{
  return IncMap[directive];
}

bool IncludeNode::Has_Parent_Marked_For_Output(void)
{
  IncludeNode *node = this;

  do {
    if (node->Should_Be_Output()) {
      return true;
    }
    node = node->Get_Parent();
  } while (node != nullptr);

  return false;
}

void IncludeNode::Mark_For_Expansion(void)
{
  IncludeNode *node = this;

  do {
    /* Mark childs for output.  */
    for (IncludeNode *child : node->Childs) {
      if (child->Should_Be_Expanded() == false && !child->Is_From_Minus_Include()) {
        child->ShouldBeExpanded = false;
        child->Mark_For_Output();
      }
    }

    /* Mark parent nodes to be expanded.  */
    if (node->ShouldBeExpanded == false) {
      node->ShouldBeExpanded = true;
      node->ShouldBeOutput = false;
    }

    node = node->Get_Parent();
  } while (node != nullptr);
}

bool IncludeNode::Is_Reachable_From_Main(void)
{
  const Preprocessor &pp = Tree.PP;
  const SourceManager &sm = Tree.SM;

  if (Is_Root()) {
    /* Root is always reachable: is the main itself.  */
    return true;
  }

  HeaderSearch &incsrch = pp.getHeaderSearchInfo();

  /* Get main file location.  We will check if the include is reachable from
     the main file.  */
  FileID main_file = sm.getMainFileID();
  SourceLocation mainfileloc = sm.getLocForStartOfFile(main_file);

  auto main_dir = ClangCompat::Get_Main_Directory_Arr(sm);

  /* If we mark a file for output, this include must be reachable from the
     main file, otherwise the generated file won't be able to find certain
     includes.  */
  OptionalFileEntryRef ref = incsrch.LookupFile(ID->getFileName(),
                                        mainfileloc,
                                        !ID->wasInQuotes(),
                                        nullptr,
                                        nullptr,
                                        main_dir,
                                        nullptr,
                                        nullptr,
                                        nullptr,
                                        nullptr,
                                        nullptr,
                                        nullptr);

  if (ref.has_value()) {
    const FileEntryRef &entry = *ref;
    const FileEntry &fentry = ref->getFileEntry();
    if (Tree.IEP->Must_Expand(fentry.tryGetRealPathName(), entry.getName())) {
      /* Lie telling it is unreachable, which would force an expansion.  */
      return false;
    }
    /* File is reachable.  */
    return true;
  } else {
    /* File not reachable.  */
    return false;
  }
}

void IncludeNode::Mark_For_Output(void)
{
  if (Is_Root()) {
    /* Root does not have an InclusionDirective.  */
    assert(ShouldBeOutput == false && "Root has output true.");
    return;
  }

  if (Is_Reachable_From_Main()) {
    /* File is reachable.  We can safely set this file to output and be done.  */
    ShouldBeOutput = true;
  } else {
    /* File not reachable.  We must expand this.  */
    Mark_For_Expansion();
  }
}

void IncludeNode::Set_HeaderGuard(MacroDefinitionRecord *guard)
{
  if (HeaderGuard == nullptr) {
    HeaderGuard = guard;
  } else {
    std::string message = "Attempt to redefine headerguard of " + Get_Filename().str();
    DiagsClass::Emit_Warn(message, guard->getSourceRange());
  }
}

void IncludeTree::IncludeNode::Dump_Single_Node(llvm::raw_ostream &out)
{
  out << File->getName() << " Expand: " << ShouldBeExpanded <<
         " Output: " << ShouldBeOutput << " -include: " <<
         IsFromMinusInclude << '\n';
}

void IncludeTree::IncludeNode::Dump_Single_Node(void)
{
  Dump_Single_Node(llvm::outs());
}

void IncludeTree::IncludeNode::Dump(llvm::raw_ostream &out, unsigned ident)
{
  out << std::string(ident*2, ' ');
  Dump_Single_Node(out);

  for (IncludeNode *child : Childs) {
    child->Dump(out, ident + 1);
  }
}

void IncludeTree::IncludeNode::Dump(void)
{
  Dump(llvm::outs());
}
