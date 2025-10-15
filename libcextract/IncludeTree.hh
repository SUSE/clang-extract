//===- IncludeTree.hh - Tree of #include'd files --------------- *- C++ -*-===//
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

/** IncludeTree: Tree of include directives.
 *
 * A C or C++ source file often #include other files for prototypes of
 * functions, macros, and such to be used in the code. Those files also
 * include other files, which if we stand from the mainfile perspective
 * it can be seen as a tree. This file implement such tree.
 */

#pragma once

#include "ExpansionPolicy.hh"

#include <clang/Tooling/Tooling.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>

using namespace clang;


/** @brief Tree of include directives.
  *
  * This tree holds information of all files that ends up included in the
  * source.
  */
class IncludeTree
{
  public:

  /** @brief Node of #include representing a single file.
   *
   * A single included file may include several other files.  This class
   * represent a single file in this inclusion tree.
   */
  class IncludeNode
  {
    public:
    /** Build the IncludeNode from a single InclusionDirective taken from the
        PreprocessingDirective history.  Set if header must be marked to `output`
        or `expanded` when printed.  */
    IncludeNode(IncludeTree *tree, InclusionDirective *include,
                bool is_from_minus_inclue = false,
                bool output = false, bool expand = false,
                bool not_output = false, bool not_expand = false);

    /** Build an IncludeNode to a null file, which is not #include'd
        anywhere but is the root of everything.  */
    IncludeNode(IncludeTree *tree);

    /* Destructor.  */
    ~IncludeNode(void);

    /* Check if #include comes from -include, which is handled a little
       differently.  */
    inline bool Is_From_Minus_Include(void)
    {
      return IsFromMinusInclude;
    }

    /** Get file which includes this file.  If this is included in multiple
        locations you will have to transverse the tree to find the other
        occurence of the same file, once macros can control which part of the
        file is included or not, drastically changing its behavior.  */
    IncludeNode *Get_Parent(void);

    /** Get the i-th file that this file includs.  */
    IncludeNode *Get_Child(unsigned i);

    /** Get the number of includes in this file.  */
    unsigned Get_Num_Childs(void);

    /** Get all files this file includes.  */
    std::vector<IncludeTree::IncludeNode *> *Get_Childs(void);

    /* Get reference to FileEntry object.  Used to check if two IncludeNodes
       references the same file.  */
    OptionalFileEntryRef Get_FileEntry(void);

    /** Check if two IncludeNodes references the same file.  */
    bool Is_Same_File(IncludeNode *);
    bool Is_Same_File(OptionalFileEntryRef file);

    /** Get a range in source location which belongs to the file included.  */
    SourceRange Get_File_Range(void);

    /** Get a range in source location which belongs to the #include, as the
        user spelled.  */
    SourceRange Get_Include_Spelling_Range(void);

    /** Get Filename.  */
    StringRef Get_Filename(void);

    inline InclusionDirective *Get_InclusionDirective(void)
    {
      return ID;
    }

    inline bool Should_Be_Output(void)
    {
      return ShouldBeOutput;
    }

    inline bool Should_Be_Expanded(void)
    {
      return ShouldBeExpanded;
    }

    inline bool Should_Not_Be_Expand(void)
    {
      return ShouldNotBeExpanded;
    }

    inline bool Should_Not_Be_Output(void)
    {
      return ShouldNotBeOutput;
    }

    inline MacroDefinitionRecord *Get_HeaderGuard(void)
    {
      return HeaderGuard;
    }

    /** Check if this node is reachable from the main file, i.e., including it
        in the program would not result in a file not found error.  */
    bool Is_Reachable_From_Main(void);

    /** Check if this node or any of its parents was marked for output.  */
    bool Has_Parent_Marked_For_Output(void);

    inline bool Is_Root(void)
    {
      return this == Tree.Root;
    }

    /** Check if the node can be marked for expansion.  */
    bool Can_Be_Expanded(void);

    /** Dump, for debugging reasons.  */
    void Dump_Single_Node(llvm::raw_ostream &out);
    void Dump(llvm::raw_ostream &out, unsigned ident = 0);

    void Dump_Single_Node(void);
    void Dump(void);

    private:

    /** Add a #include to this file.  */
    void Add_Child(IncludeNode *);

    /** Set that this file was #include'd by that file.  */
    void Set_Parent(IncludeNode *that);

    /* Set the File referenced.  Used when building the tree.  */
    void Set_FileEntry(OptionalFileEntryRef file);

    /* Set the macro defintion as the HeaderGuard.  */
    void Set_HeaderGuard(MacroDefinitionRecord *guard);

    /** The Tree this node belongs to.  */
    IncludeTree &Tree;

    /** Object from the PreprocessingRecord representing an #include.  */
    InclusionDirective *ID;

    /** Referece to the actual #include'd file.  */
    OptionalFileEntryRef File;

    /** Macro definition which happens to be the headerguard for this header.  */
    MacroDefinitionRecord *HeaderGuard;

    bool ShouldBeOutput : 1;
    bool ShouldBeExpanded : 1;
    bool ShouldNotBeOutput : 1;
    bool ShouldNotBeExpanded : 1;
    bool IsFromMinusInclude : 1;

    /** Who included me?  */
    IncludeNode *Parent;

    /** Which files did I include?  */
    std::vector<IncludeNode *> Childs;

    friend IncludeTree;
  };

  /** Create the include tree from the Preprocessor history.  */
  IncludeTree(Preprocessor &pp, SourceManager &sm,
              IncludeExpansionPolicy::Policy p = IncludeExpansionPolicy::Policy::NOTHING,
              std::vector<std::string> const &must_expand = {},
              std::vector<std::string> const &must_not_expand = {});

  IncludeTree(ASTUnit *ast,
              IncludeExpansionPolicy::Policy p = IncludeExpansionPolicy::Policy::NOTHING,
              std::vector<std::string> const &must_expand = {},
              std::vector<std::string> const &must_not_expand = {})
    : IncludeTree(ast->getPreprocessor(), ast->getSourceManager(), p,
                  must_expand, must_not_expand)
  {
  }

  /** Default destructor.  */
  ~IncludeTree(void);

  /** Get includes which is not marked to be expanded.  */
  std::unique_ptr<std::vector<IncludeNode *>> Get_Non_Expand_Includes(void);

  /** Get all includes.  */
  std::unique_ptr<std::vector<IncludeNode *>> Get_Includes(void);

  /* Get from SourceLocation.  */
  IncludeNode *Get(const SourceLocation &loc);

  /** Get from InclusionDirective.  */
  IncludeNode *Get(const InclusionDirective *);

  /** Get from FileEntry.  */
  IncludeNode *Get(const FileEntry *);

  /** Dump for debugging purposes.  */
  void Dump(llvm::raw_ostream &out);
  void Dump(void);

  private:

  /** Actually builds the header tree.  */
  void Build_Header_Tree(std::vector<std::string> const &must_expand,
                         std::vector<std::string> const &must_not_expand);
  /** Build mapping from header path to IncludeNode.  */
  void Build_Header_Map(void);

  /** Set the ShouldBeExpanded flag after the barebones of the tree is built.  */
  void Set_Expansion_Attrs(IncludeNode *node);

  /** Set the ShouldBeOutput flag after the barebones of the tree is built
      and the ShouldBeOutput flag is correctly set.  */
  void Set_Output_Attrs(void);

  void Cut_Non_Expandable_Trees(IncludeNode *, bool cutting);

  enum PolicyRule {
    MUST_EXPAND,
    MUST_NOT_EXPAND,
  };

  /** Run the IncludeExpansionPolicy to the include to figure out if it needs
      to be expanded or not.  */
  bool Run_Expansion_Policy(InclusionDirective *ID, SourceLocation incloc, PolicyRule p);

  /** Run the Must_Expand rule of IncludeExpansionPolicy to the include to
      figure out if it needs to be expanded or not.  */
  inline bool Run_Must_Expand_Policy(InclusionDirective *ID)
  {
    return Run_Expansion_Policy(ID, ID->getSourceRange().getBegin(), MUST_EXPAND);
  }

  /** Run the Must_Not_Expand rule of IncludeExpansionPolicy to the include to
      figure out if it can be expanded or not.  */
  inline bool Run_Must_Not_Expand_Policy(InclusionDirective *ID)
  {
    return Run_Expansion_Policy(ID, ID->getSourceRange().getBegin(), MUST_NOT_EXPAND);
  }

  /** Check if this node is reachable from the main file, i.e., including it
      in the program would not result in a file not found error.  */
  bool Is_Reachable_From_Main(InclusionDirective *id);

  /** Root of the tree.  */
  IncludeNode *Root;

  /* Hash mapping included file to IncludeNode.  */
  std::unordered_map<const FileEntry *, IncludeTree::IncludeNode *> Map;
  std::unordered_map<const InclusionDirective *, IncludeTree::IncludeNode *> IncMap;

  /** Reference to the preprocessor used by compilation.  */
  Preprocessor &PP;

  /** Reference to the SourceManager.  */
  SourceManager &SM;

  /** The Include Expansion Policy when expanding includes.  */
  std::unique_ptr<IncludeExpansionPolicy> IEP;
};

typedef IncludeTree::IncludeNode IncludeNode;
