#pragma once

#include "EnumConstTbl.hh"

#include <clang/Tooling/Tooling.h>
#include <clang/Analysis/CallGraph.h>
#include <unordered_set>

using namespace clang;

/** Function Dependency Finder.
 *
 * This class wraps all code necessary to find all Decl in the
 * Abstract Syntax Tree that is necessary for a given function to
 * compile.
 *
 * As example, assume we have the following source code:
 *
 * int f();
 * int g()
 * {
 *   f();
 * }
 *
 * int h()
 * {
 * }
 *
 *
 * If we want to extract function `g`, we have to extract `f` as well, once it
 * is called by `g`. OTOH, if we want to only extract `h`, we don't need to do
 * anything else since this function doesn't call anything and can be compiled
 * by itself.
 *
 */
class FunctionDependencyFinder
{
  public:
    FunctionDependencyFinder(ASTUnit *ast, std::string const &function);
    FunctionDependencyFinder(ASTUnit *ast, std::vector<std::string> const &functions);

    /** Print the marked nodes as they appear in the AST.  */
    void Print();

  protected:
    /** Run the analysis on function `function`*/
    void Run_Analysis(std::vector<std::string> const &function);

    /** Mark decl as dependencies and all its previous decls versions.  */
    bool Add_Decl_And_Prevs(Decl *decl);

    /** Given function with name `funcname`, recursively find which functions
        are required by such function.  */
    void Find_Functions_Required(CallGraph *, std::vector<std::string> const &funcnames);

    /** Mark function represented by `node` and all its callees.  */
    void Mark_Required_Functions(CallGraphNode *node);

    /** Find all Types and Global variables that are reachable from
        Dependencies function.  */
    void Compute_Closure(void);

    /** Mark all types reachable from a function body or statement chain.  */
    void Mark_Types_In_Function_Body(Stmt *stmt);

    /** Add Type to the list of dependencies. Types are objects that in
        some way represent a variable type.  For example, a type can be
        a RecordDecl (struct, union), a enum, a TypedefNamedDecl (typedefs)
        or even complicated function pointers.  */
    bool Add_Type_And_Depends(const Type *);

    /* Add TypeDecl to the list of dependencies.  */
    bool Handle_TypeDecl(TypeDecl *decl);

  /* Handle the corner case where an array was declared as something like

    enum {
      const1 = 1,
    };

    int var[const1];

    in which clang will unfortunatelly expands const1 to 1 and lose the
    EnumConstantDecl in this case, forcing us to reparse this declaration.  */
    bool Handle_Array_Size(ValueDecl *decl);

    /** Datastructure holding all Decls required for the functions. This is
        then used to mark which Decls we need to output.

        We use a unordered_set because we need quick lookup, so a hash table
        may be (?) the ideal datastructure for this.  */
    std::unordered_set<Decl*> Dependencies;

    /** The AST that are being used in our analysis.  */
    ASTUnit* AST;

    /** Object holding information about constant enum symbols and a mapping to
        they original enum object.  */
    EnumConstantTable EnumTable;


    /** Check if a given declaration was already marked as dependency.  */
    inline bool Is_Decl_Marked(Decl *decl)
    { return Dependencies.find(decl) != Dependencies.end(); }
};
