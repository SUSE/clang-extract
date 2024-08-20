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
                bool output = true, bool expand = false,
                bool is_from_minus_inclue = false);

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

    inline MacroDefinitionRecord *Get_HeaderGuard(void)
    {
      return HeaderGuard;
    }

    bool Is_Reachable_From_Main(void);

    void Mark_For_Expansion(void);

    void Mark_For_Output(void);

    /** Check if this node or any of its parents was marked for output.  */
    bool Has_Parent_Marked_For_Output(void);

    inline bool Is_Root(void)
    {
      return this == Tree.Root;
    }

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
              std::vector<std::string> const &must_expand = {});

  IncludeTree(ASTUnit *ast,
              IncludeExpansionPolicy::Policy p = IncludeExpansionPolicy::Policy::NOTHING,
              std::vector<std::string> const &must_expand = {})
    : IncludeTree(ast->getPreprocessor(), ast->getSourceManager(), p, must_expand)
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

  /** Dump for debugging purposes.  */
  void Dump(llvm::raw_ostream &out);
  void Dump(void);

  private:

  /** Actually builds the header tree.  */
  void Build_Header_Tree(std::vector<std::string> const &must_expand);

  /** Build mapping from header path to IncludeNode.  */
  void Build_Header_Map(void);

  /** Fix the case where the IncludeNode can not be marked to output and needs
      to be expanded because it is not reachable from the main file.  */
  void Fix_Output_Attrs(IncludeNode *node);

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
