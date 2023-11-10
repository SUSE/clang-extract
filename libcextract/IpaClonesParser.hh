/** IpaClonesParser: Parse GCC IPA-clones files and build an inline graph.
  *
  * GCC performs many cross-function optimizations, one of them is `inlining`,
  * which copies the body of the callee into the caller. This optimization often
  * removes the original symbol from the ELF file and insert additional stuff
  * into the caller, so that if we want to livepatch this function we need to be
  * careful about them. This file implements the datastructures necessary for
  * parsing and processing those files.
  */

#pragma once

#include "Parser.hh"

#include <set>
#include <unordered_map>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <memory>

/** @brief Represent an cloned node.  See this as a callgraph, but we only
  *  incide edges when there is an inline.
  */
struct IpaCloneNode
{
  /** Name of the symbol.  */
  std::string Name;
  /** Where is this symbol inlined into?  */
  std::set<IpaCloneNode *> InlinedInto;

  /** Which symbols this symbol is inline into?  */
  std::set<IpaCloneNode *> Inlines;
};

/** @brief Parse .ipa-clone files and build an inline graph.  */
class IpaClones : public Parser
{
  public:
  /** There are two decision cases:
    * - REMOVE, which removes the function from the code.
    * - CLONE, which does some transformation in the code.
    *
    * This is used to find out how many tokens there are in a single line of
    * the .ipa-clones file.
    */
  enum IpaDecision
  {
    IPA_REMOVE,
    IPA_CLONE,
  };

  /** Construct the IpaClones from a ipa-clones files.  */
  IpaClones(const char *path);

  // IPA clones can point to a directory, so we need to handle paths at this
  // point.
  void Parse(const char *path);

  /** Construct the IpaClones from a ipa-clones files.  */
  inline IpaClones(const std::string &path)
    : IpaClones(path.c_str())
  { }

  /** Get a node with matching ASM name.  ASM names are unique even for C++ so
    * there should not be any clashes.  */
  inline IpaCloneNode *Get_Node(const std::string &name)
  {
    std::unordered_map<std::string, IpaCloneNode>::iterator it = Nodes.find(name);
    if (it != Nodes.end()) {
      return &Nodes[name];
    }

    return nullptr;
  }

  /** Get a node with matching ASM name.  ASM names are unique even for C++ so
    * there should not be any clashes.  */
  inline IpaCloneNode *Get_Node(const char *name)
  {
    return Get_Node(std::string(name));
  }

  /** Get a node with matching ASM name, or create it if does not exists.  */
  IpaCloneNode *Get_Or_Create_Node(const std::string &name);

  /** Get a node with matching ASM name, or create it if does not exists.  */
  inline IpaCloneNode *Get_Or_Create_Node(const char *name)
  {
    return Get_Or_Create_Node(std::string(name));
  }

  /** Iterate through the set of all nodes.  */
  inline std::unordered_map<std::string, IpaCloneNode>::iterator begin()
  {
    return Nodes.begin();
  }
  inline std::unordered_map<std::string, IpaCloneNode>::iterator end()
  {
    return Nodes.end();
  }

  void Dump(void);
  void Dump_Graphviz(const char *filename);

  private:

  void Open_Recursive(const char *path);

  /** Set of nodes.  */
  std::unordered_map<std::string, IpaCloneNode> Nodes;

  /** Hold the state machine of the lexer, which is where in the string it is.  */
  class LexingState
  {
    public:
    inline LexingState(char *line)
      : CurrentStateString(line),
        OriginalPtr(line)
    {
    }

    inline ~LexingState(void)
    {
      free(OriginalPtr);
    }

    /** Next token in file.  */
    const char *Lex(void);

    /** Parse the decision string into a IpaDecision.  */
    static IpaDecision Get_Decision(const char *);

    /** Current state for strtok.  */
    char *CurrentStateString;

    /** Original string returned by getline.  */
    char *OriginalPtr;
  };
};

/** Compute the closure of a symbol executing an Action on each edge.
  *
  * On many analysis we need to do an DFS on the IPA-clones graph and execute
  * a set of commands on each edge of the graph.  This class abstract this by
  * giving a way to pass a function pointer and a opaque pointer to an object
  * which can be used to populate with information.
  */
struct IpaClosure
{
  public:
  /** Build the object.  
    * @param ipa     The IPA clones object.
    * @param opaque  Pointer to an opaque object handled by `action_function`.
    * @param action_function Function which will do something with the `opaque` and
    *                        nodes given to it.
    */
  inline IpaClosure(IpaClones &ipa, void *opaque,
                    int (*action_function)(void *, IpaCloneNode *, IpaCloneNode *))
    : Ipa(ipa),
      Opaque(opaque),
      Action(action_function)
  {
  }

  /** Find which symbols are inlined in the function represented by `node`.  */
  void Find_Inlined_Symbols(IpaCloneNode *node);

  /** Find the other functions where the symbol represented by `node` is
      inlined.  */
  void Find_Where_Symbol_Is_Inlined(IpaCloneNode *node);

  /** Find which symbols are inlined in the function with mangled name `name`.  */
  inline void Find_Inlined_Symbols(const std::string &name)
  {
    IpaCloneNode *node = Ipa.Get_Node(name);
    Find_Inlined_Symbols(node);
  }

  /** Find the other functions where the symbol with mangled name `name` is
      inlined.  */
  inline void Find_Where_Symbol_Is_Inlined(const std::string &name)
  {
    IpaCloneNode *node = Ipa.Get_Node(name);
    Find_Where_Symbol_Is_Inlined(node);
  }

  /** Set of marked symbols.  */
  std::set<IpaCloneNode *> Set;

  private:

  /** Is symbol marked? (already analyzed).  */
  inline bool Is_In_Set(IpaCloneNode *x)
  {
    return Set.find(x) != Set.end();
  }

  /** Reference to the IpaClones object used to build this object.  */
  IpaClones &Ipa;

  /** Opaque pointer that is always passed to Action.  */
  void *Opaque;

  /** Action function that will always be executed on each edge.  */
  int (*Action)(void *opaque, IpaCloneNode *, IpaCloneNode *);
};
