#pragma once

#include <clang/Tooling/Tooling.h>
#include <clang/Analysis/CallGraph.h>
#include <vector>
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
    FunctionDependencyFinder(std::unique_ptr<ASTUnit> ast, std::string const &function);

    /** Print the marked nodes as they appear in the AST.  */
    void Print();

  private:
    /** Run the analysis on function `function`*/
    void Run_Analysis(std::string const &function);

    /** Mark decl as dependencies and all its previous decls versions.  */
    bool Add_Decl_And_Prevs(Decl *decl);

    /** Given function with name `funcname`, recursively find which functions
        are required by such function.  */
    void Find_Functions_Required(CallGraph *, std::string const &funcname);

    /** Mark function represented by `node` and all its callees.  */
    void Mark_Required_Functions(CallGraphNode *node);

    /** Find all RecordDecls that are reachable from Dependencies function.  */
    void Find_Records_Required(void);

    void Mark_Records_In_Function_Body(Stmt *stmt);

    bool Add_TagDecl_And_Nested(TagDecl *);

    /** Datastructure holding all Decls required for the functions. This is
        then used to mark which Decls we need to output.

        We use a unordered_set because we need quick lookup, so a hash table
        may be (?) the ideal datastructure for this.  */
    std::unordered_set<Decl*> Dependencies;

    /* The AST that are being used in our analysis.  */
    std::unique_ptr<ASTUnit> AST;

    inline bool Is_Decl_Marked(Decl *decl)
    { return Dependencies.find(decl) != Dependencies.end(); }
};
