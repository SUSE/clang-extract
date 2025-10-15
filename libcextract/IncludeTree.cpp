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

static bool In_Set(std::unordered_set<std::string> &set, const StringRef &str,
                   bool remove_if_exists)
{
  return In_Set(set, str.str(), remove_if_exists);
}

/* ----- IncludeTree ------ */
IncludeTree::IncludeTree(Preprocessor &pp,
                         SourceManager &sm,
                         IncludeExpansionPolicy::Policy p,
                         std::vector<std::string> const &must_expand,
                         std::vector<std::string> const &must_not_expand)
  : PP(pp),
    SM(sm),
    IEP(IncludeExpansionPolicy::Get_Expansion_Policy_Unique(p))
{
  /* First step: create the barebones IncludeTree structure.  If a node is
     set to expansion, or not expansion, or can't be output because it is
     unreachable from the main file, it is flagged here.  */
  Build_Header_Tree(must_expand, must_not_expand);

  /* Second step: propagate the ShouldBeExpanded flag upwards on the tree,
     which means sweeping through the tree post-order transversal to
     propagate that flag upward.  */
  Set_Expansion_Attrs(Root);

  /* Third step: now set and propagate the "ShouldBeOutput flag downards the
     tree, iterating it using pre-order transversal.  In the case a header
     attempts to be set to be output but it would violate the
     ShouldNotBeOutput flag, then it set the node for expansion and try
     to set its children for output.  */
  Set_Output_Attrs();

  /* Forth step: for headers that must NOT be expanded, we cut it from the
     tree, which means we cancel out the 'expansion' flag even if the user
     requested.  In case the header cannot be set to output, we error out.  */
  Cut_Non_Expandable_Trees(Root, false);

  /* Create a hash of FileEntry => IncludeNode for fast lookup.  */
  Build_Header_Map();
}

IncludeTree::~IncludeTree(void)
{
  delete Root;
  Root = nullptr;
}

void IncludeTree::Build_Header_Tree(std::vector<std::string> const &must_expand,
                                    std::vector<std::string> const &must_not_expand)
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
  std::unordered_set<std::string> must_not_expand_set(must_not_expand.begin(),
                                                      must_not_expand.end());

  MacroWalker mw(PP);
  bool already_seen_main = false;
  OptionalFileEntryRef main = SM.getFileEntryRefForID(SM.getMainFileID());

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

      /* Check if the user or SymbolExternalizer requested a specific header to be
         expanded.  */
      bool expand = In_Set(must_expand_set, id->getFileName(), /*remove=*/true) ||
                    In_Set(must_expand_set, id->getFile()->getName(), /*remove=*/true);

      /* Starting from LLVM-18 and in some platforms (s390x), the behaviour of
       * FileEntry::getName() changed. Now it returns the full path of the file
       * rather than the relative path to it, so here we account it for now onwards.
       */
      expand |= In_Set(must_expand_set,
                       id->getFile()->getFileEntry().tryGetRealPathName(),
                       /*remove=*/true);

      /* Check if the policy will request this header to be expanded.  */
      bool policy_expand = Run_Must_Expand_Policy(id);

      bool not_expand = In_Set(must_not_expand_set, id->getFileName(), false) ||
                        In_Set(must_not_expand_set, id->getFile()->getName(), false) ||
                        In_Set(must_not_expand_set,
                               id->getFile()->getFileEntry().tryGetRealPathName(), false);

      /* Now check what the Must_Not_Expand policy will say.  If we have a conflict
         with what the user or SymbolExternalizer is saying, we cannot continue.  */
      bool policy_not_expand = Run_Must_Not_Expand_Policy(id);

      /* Check if its sane.  If the user (or SymbolExternalizer) requested that
         a must-not-expand header to be expanded, then error out.  */
      if (expand && policy_not_expand) {
        std::string message = "Unable to expand header " + id->getFileName().str() +
        ": denied by ExpansionPolicy.";
        DiagsClass::Emit_Error(message, id->getSourceRange(), SM);
        throw std::runtime_error("IncludeTree is unable to continue");
        return;
      }

      if (expand && not_expand) {
        std::string message = "Unable to expand header " + id->getFileName().str() +
        ": denied by -DCE_NOT_EXPAND_INCLUDES.";
        DiagsClass::Emit_Error(message, id->getSourceRange(), SM);
        throw std::runtime_error("IncludeTree is unable to continue");
        return;
      }

      bool can_be_output = Is_Reachable_From_Main(id);
      bool is_from_minus_include = !already_seen_main;

      /* Add child to tree.  */
      IncludeNode *child = new IncludeNode(this, id, is_from_minus_include,
                                           false, expand | policy_expand,
                                           !can_be_output,
                                           not_expand | policy_not_expand);

      current->Add_Child(child);
      child->Set_Parent(current);

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
                              " Only the first is registered.", range, SM);
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

void IncludeTree::Set_Expansion_Attrs(IncludeNode *node)
{
  /* Because we need to propagate the ShouldBeExpanded upwards in the tree,
     we iterate on it using post-order transversal.  */
  for (IncludeNode *child : node->Childs) {
    Set_Expansion_Attrs(child);
  }

  /* We must not touch root.  */
  if (node->Is_Root())
    return;

  if (node->ShouldBeExpanded) {
    IncludeNode *parent = node->Get_Parent();
    parent->ShouldBeExpanded = true;
    parent->ShouldBeOutput = false;
  }
}

void IncludeTree::Set_Output_Attrs(void)
{
  /* Because we need to propagate the ShouldBeOutput downwards in the tree,
     we iterate on it using pre-order transversal by the aid of an stack.  */
  std::stack<IncludeNode *> stack;
  stack.push(Root);
  while (!stack.empty()) {
    IncludeNode *node = stack.top(); stack.pop();
    IncludeNode *parent = node->Get_Parent();

    /* In case the parent is set to be expanded, we must output this
       include.  */
    if (parent && parent->ShouldBeExpanded && !node->ShouldBeExpanded) {
      if (!node->Is_From_Minus_Include() || !parent->Is_Root()) {
        /* In the case this node can not be output, we need set it to
           expansion and try later for its children.  */
        if (node->ShouldNotBeOutput) {
          node->ShouldBeExpanded = true;
        } else {
          node->ShouldBeOutput = true;
        }
      }
    }

    /* Add children to the stack.  */
    for (IncludeNode *child : node->Childs) {
      stack.push(child);
    }
  }
}

void IncludeTree::Cut_Non_Expandable_Trees(IncludeNode *node, bool cutting)
{
  /* Because we need to propagate the ShouldBeOuput downwards in the tree,
     we iterate on it using pre-order transversal recursively.  */

  /* If we are already cutting it from the tree.  */
  if (cutting) {
    node->ShouldBeOutput = false;
    node->ShouldBeExpanded = false;
  } else if (node->ShouldNotBeExpanded) {
    /* We need to start cutting it from the tree.  */
    node->ShouldBeOutput = true;
    node->ShouldBeExpanded = false;

    if (node->ShouldNotBeOutput) {
      std::string message = "Unable to not expand header " +
                            node->ID->getFileName().str() +
                            ": header is not reachable from main file.";
      DiagsClass::Emit_Error(message, node->ID->getSourceRange(), SM);
      throw std::runtime_error("IncludeTree is unable to continue");
    }
  }

  /* Propagate downards.  */
  bool cut = cutting || node->ShouldNotBeExpanded;
  for (IncludeNode *child : node->Childs) {
    Cut_Non_Expandable_Trees(child, cut);
  }
}

IncludeNode *IncludeTree::Get(const SourceLocation &loc)
{
  OptionalFileEntryRef fileref = PrettyPrint::Get_FileEntry(loc);

  /* In case we could not find a FileRef, then try the ExpansionLoc.  */
  if (!fileref.has_value()) {
    const SourceLocation &loc2 = SM.getExpansionLoc(loc);
    fileref = PrettyPrint::Get_FileEntry(loc2);
  }

  if (fileref.has_value()) {
    return Get(&fileref->getFileEntry());
  }

  /* Well, the declaration seems fuzzy, so return that we don't have a file.  */
  return nullptr;
}

IncludeNode *IncludeTree::Get(const FileEntry *file)
{
  return Map[file];
}

bool IncludeTree::Is_Reachable_From_Main(InclusionDirective *ID)
{
  HeaderSearch &incsrch = PP.getHeaderSearchInfo();

  /* Get main file location.  We will check if the include is reachable from
     the main file.  */
  FileID main_file = SM.getMainFileID();
  SourceLocation mainfileloc = SM.getLocForStartOfFile(main_file);

  auto main_dir = ClangCompat::Get_Main_Directory_Arr(SM);

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
    /* File is reachable.  */
    return true;
  } else {
    /* File not reachable.  */
    return false;
  }
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
                                      bool is_from_minus_include,
                                      bool output, bool expand,
                                      bool not_output, bool not_expand)
  : Tree(*tree),
    ID(include),
    File(ID->getFile()),
    HeaderGuard(nullptr),
    ShouldBeOutput(output),
    ShouldBeExpanded(expand),
    ShouldNotBeOutput(not_output),
    ShouldNotBeExpanded(not_expand),
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
    ShouldNotBeOutput(false),
    ShouldNotBeExpanded(false),
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
  SourceLocation start, end;
  SourceManager &SM = Tree.SM;
  FileID fid = SM.getOrCreateFileID(*File, SrcMgr::CharacteristicKind());
  start = SM.getLocForStartOfFile(fid);
  end = SM.getLocForEndOfFile(fid);

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

bool IncludeTree::Run_Expansion_Policy(InclusionDirective *ID,
                                       SourceLocation inloc, PolicyRule p)
{
  auto main_dir = ClangCompat::Get_Main_Directory_Arr(SM);
  HeaderSearch &incsrch = PP.getHeaderSearchInfo();

  /* If we mark a file for output, this include must be reachable from the
     main file, otherwise the generated file won't be able to find certain
     includes.  */
  OptionalFileEntryRef ref = incsrch.LookupFile(ID->getFileName(),
                                        inloc,
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
    if (p == PolicyRule::MUST_EXPAND) {
      return IEP->Must_Expand(fentry.tryGetRealPathName(), entry.getName());
    } else if (p == PolicyRule::MUST_NOT_EXPAND) {
      return IEP->Must_Not_Expand(fentry.tryGetRealPathName(), entry.getName());
    } 
  }

  return false;
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

bool IncludeNode::Is_Reachable_From_Main(void)
{
  if (Is_Root()) {
    /* Root is always reachable: is the main itself.  */
    return true;
  }

  return Tree.Is_Reachable_From_Main(ID);
}

void IncludeNode::Set_HeaderGuard(MacroDefinitionRecord *guard)
{
  if (HeaderGuard == nullptr) {
    HeaderGuard = guard;
  } else {
    std::string message = "Attempt to redefine headerguard of " + Get_Filename().str();
    DiagsClass::Emit_Warn(message, guard->getSourceRange(), Tree.SM);
  }
}

bool IncludeTree::IncludeNode::Can_Be_Expanded(void)
{
  IncludeNode *node = this;
  while (node) {
    if (node->ShouldNotBeExpanded) {
      return false;
    }
    node = node->Get_Parent();
  }

  return true;
}

void IncludeTree::IncludeNode::Dump_Single_Node(llvm::raw_ostream &out)
{
  out << File->getName() << " Expand: " << ShouldBeExpanded <<
         " Output: " << ShouldBeOutput << " NotExpand: " << ShouldNotBeExpanded <<
         " NotOutput: " << ShouldNotBeOutput << " -include: " <<
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
