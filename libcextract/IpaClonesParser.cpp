#include "IpaClonesParser.hh"
#include "NonLLVMMisc.hh"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdexcept>
#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>

/* GCC can either:
    - Change the call ABI of a function to either reduce the stack
      usage, remove instructions, or anything more complicated. The
      pass that can do it is named "Interprocedural Scalar Replacement
      of Aggregates (ISRA)", and symbols with the suffix ".isra.%d" are
      generated from this pass. Althrough those symbols are exported and
      can be called, doing so seems not reliable.  Instead we model
      such functions as being `inlined` into the caller, forcing
      clang-extract to issue a copy of this function into the livepatch.

    - Inline part of the function in the caller.  Functions like this
      are suffixed as `.part.%d` and is again not safe to call either.
      So we model those functions as being inlined, forcing clang-extract
      to issue a copy of it into the livepatch.  */
static const char *Handle_GCC_Symbol_Quirks(char *symbol)
{
  char *first_dot = strchr(symbol, '.');
  if (first_dot == nullptr) {
    return symbol;
  }

  if (prefix(".part.", first_dot)) {
    /* Symbol generated by ipa-split.c pass.  Erase the .part.  */
    *first_dot = '\0';
    return symbol;
  }

  if (prefix(".isra.", first_dot)) {
    /* Symbol generated by ipa-isra.c pass.  Erase the .isra.  */
    *first_dot = '\0';
    return symbol;
  }

  return symbol;
}

IpaClones::IpaClones(const char *path)
    : Parser(path)
{
  if (Is_Directory(path) == false) {
    /* Single file.  We can pass it directly to Parse.  */
    Parse(path);
    return;
  }

  Open_Recursive(path);
}

void IpaClones::Open_Recursive(const char *path)
{
  /* Else we have to parse the directory tree.  */
  DIR *directory = opendir(path);
  struct dirent *entry;
  char buffer[PATH_MAX];

  if (directory == nullptr) {
    /* Is a directory but isn't?  */
    throw new std::runtime_error("Path is invalid!");
  }

  /* Copy path to buffer.  */
  size_t len = strlen(path);
  size_t size = len + 1;
  memcpy(buffer, path, size);

  /* Add the '/' directory separation token if it doesn't exists.  */
  if (buffer[len-1] != '/') {
    buffer[len++] = '/';
    buffer[len] = '\0';
    size++;
  }

  char *buffer_after_slash = &buffer[len];

  /* Iterate on each file of directory.  */
  while ((entry = readdir(directory)) != nullptr) {
    const char *file = entry->d_name;
    if (strcmp(file, ".") == 0 || strcmp(file, "..") == 0) {
      /* Skip the current and previous directory to avoid infinite loop.  */
      continue;
    }

    /* If extension matches, then analyze this file.  */
    const char *extension = strrchr(file, '.');
    if (extension && strcmp(extension, ".ipa-clones") == 0) {
      strcpy(buffer_after_slash, file);
      Parse(buffer);
      continue;
    }

    /* If path is a directory, then analyze it recursively.  */
    strcpy(buffer_after_slash, file);
    if (Is_Directory(buffer)) {
      Open_Recursive(buffer);
    }
  }
  closedir(directory);
}

const char *IpaClones::LexingState::Lex(void)
{
  char *str = strtok(CurrentStateString, ";");

  /* Passing the nullptr to strtok on next iteration results in it calling
     strtok into the correct offset of the original pointer.  For more info,
     see `man strtok`.  */
  CurrentStateString = nullptr;
  return str;
}

IpaClones::IpaDecision IpaClones::LexingState::Get_Decision(const char *str)
{
  if (!strcmp(str, "Callgraph removal")) {
    return IPA_REMOVE;
  }

  if (!strcmp(str, "Callgraph clone")) {
    return IPA_CLONE;
  }

  /* Should never happen.  */
  __builtin_unreachable();
}

IpaCloneNode *IpaClones::Get_Or_Create_Node(const std::string &name)
{
  IpaCloneNode *ret = Get_Node(name);
  if (ret) {
    return ret;
  }

  /* Not found.  Create it.  */
  IpaCloneNode node;
  node.Name = name;
  Nodes[name] = node;

  return &Nodes[name];
}

void IpaClones::Parse(const char *path)
{
  FILE *file = fopen(path, "r");
  if (file == nullptr) {
    throw new std::runtime_error("File not found!");
  }

  char *line;

  /** Based on the following function, extracted from GCC source:
    *
    *  void
    *  dump_callgraph_transformation (const cgraph_node *original,
    *               const cgraph_node *clone,
    *               const char *suffix)
    *  {
    *    if (symtab->ipa_clones_dump_file)
    *      {
    *        fprintf (symtab->ipa_clones_dump_file,
    *           "Callgraph clone;%s;%d;%s;%d;%d;%s;%d;%s;%d;%d;%s\n",
    *           original->asm_name (), original->order,
    *           DECL_SOURCE_FILE (original->decl),
    *           DECL_SOURCE_LINE (original->decl),
    *           DECL_SOURCE_COLUMN (original->decl), clone->asm_name (),
    *           clone->order, DECL_SOURCE_FILE (clone->decl),
    *           DECL_SOURCE_LINE (clone->decl), DECL_SOURCE_COLUMN (clone->decl),
    *           suffix);
    *
    *        symtab->cloned_nodes.add (original);
    *        symtab->cloned_nodes.add (clone);
    *      }
    *  }
    */

  while ((line = getline_easy(file)) != nullptr) {
    LexingState lexer(line);
    IpaDecision decision = IpaClones::LexingState::Get_Decision(lexer.Lex());

// The IPA files contain things that we do not use but we need to parse them
// anyway.  So disable the warning for those unused variables.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
    const char *original_asm_name = lexer.Lex();
    long original_order = atol(lexer.Lex());
    const char *original_filename = lexer.Lex();
    unsigned original_line = atol(lexer.Lex());
    unsigned original_column = atol(lexer.Lex());

    if (decision == IPA_CLONE) {
      const char *clone_asm_name = lexer.Lex();
      long clone_order = atol(lexer.Lex());
      const char *clone_filename = lexer.Lex();
      unsigned clone_line = atol(lexer.Lex());
      unsigned clone_column = atol(lexer.Lex());
#pragma GCC diagnostic pop

      const char *happened = lexer.Lex();
      if (!strcmp(happened, "inlining to")) {
        const char *cleaned_caller_name = Handle_GCC_Symbol_Quirks((char*)clone_asm_name);
        const char *cleaned_callee_name = Handle_GCC_Symbol_Quirks((char*)original_asm_name);

        /* Inlining a symbol to itself makes no sense.  Yet this can happen if
           they were actually two symbols that we merged into one on
           Handle_GCC_Symbol_Quirks.  */
        if (strcmp(cleaned_callee_name, cleaned_caller_name) == 0) {
          continue;
        }

        /* This node has been inlined into.  */
        IpaCloneNode *callee = Get_Or_Create_Node(cleaned_callee_name);
        IpaCloneNode *caller = Get_Or_Create_Node(cleaned_caller_name);

        callee->InlinedInto.insert(caller);
        caller->Inlines.insert(callee);
      }
    }
  }
  fclose(file);
}

void IpaClones::Dump(void)
{
  for (auto &p : Nodes) {
    bool has_content = false;
    std::cout << " " << p.first;
    for (auto &q : p.second.InlinedInto) {
      if (has_content == false) {
        has_content = true;
        std::cout << " => ";
      }
      std::cout << q->Name << "  ";
    }
    std::cout << '\n';
  }
}

void IpaClones::Dump_Graphviz(const char *filename)
{
  FILE *file = fopen(filename, "w");
  if (file == nullptr) {
    return;
  }

  fprintf(file, "strict digraph {");
  for (auto &p : Nodes) {
    for (auto &q : p.second.InlinedInto) {
      fprintf(file, "\n\"%s\" -> \"%s\"", p.first.c_str(), q->Name.c_str());
    }
  }
  fprintf(file, "\n}");
  fclose(file);
}

/** Find which symbols are inlined in the function represented by `node`.  */
void IpaClosure::Find_Inlined_Symbols(IpaCloneNode *node)
{
  if (node == nullptr || Is_In_Set(node)) {
    return;
  }

  /** Mark node as analyzed.  */
  Set.insert(node);

  /** Proceed to other nodes in a DFS fashion.  */
  for (IpaCloneNode *n : node->Inlines) {
    Action(Opaque, node, n);
    Find_Inlined_Symbols(n);
  }
}

void IpaClosure::Find_Where_Symbol_Is_Inlined(IpaCloneNode *node)
{
  if (node == nullptr || Is_In_Set(node)) {
    return;
  }

  /** Mark node as analyzed.  */
  Set.insert(node);

  /** Proceed to other nodes in a DFS fashion.  */
  for (IpaCloneNode *n : node->InlinedInto) {
    Action(Opaque, node, n);
    Find_Where_Symbol_Is_Inlined(n);
  }
}
