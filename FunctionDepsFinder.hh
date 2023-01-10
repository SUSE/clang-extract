#pragma once

#include "EnumConstTbl.hh"
#include "MacroWalker.hh"

#include <clang/Tooling/Tooling.h>
#include <clang/Analysis/CallGraph.h>
#include <unordered_set>

using namespace clang;

struct MacroIterator
{
  clang::PreprocessingRecord::iterator macro_it;
  unsigned undef_it;
};

/** Build CallGraph from AST.
 *
 * The CallGraph is a datastructure in which nodes are functions and edges
 * represents function call points. For example:
 *
 * void f();
 * void g() { f(); f(); }
 *
 * Resuluts in the following CallGraph:
 *
 * (f) -> (g)
 *     -> (g)
 *
 * There are two edges to `g` because there are two callpoints to it.
 * Hence, the resulting graph is not `simple` in Graph Theory nomenclature. But
 * for the analysis we are doing it is suffice to be, so perhaps some extra
 * performance can be archived if we could remove duplicated edges.
 *
 */
CallGraph *Build_CallGraph_From_AST(ASTUnit *ast);

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
    FunctionDependencyFinder(ASTUnit *ast, std::string const &function, bool closure=true);
    FunctionDependencyFinder(ASTUnit *ast, std::vector<std::string> const &functions, bool closure=true);

    /** Print the marked nodes as they appear in the AST.  */
    void Print();

  protected:
    /** Run the analysis on function `function`*/
    void Run_Analysis(std::vector<std::string> const &function, bool closure);

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

    /** Macros can expand into other macros, which must be expanded as well in a tree
        like fashion.  We don't implement such tree, so we track every macro that
        should be expanded as well by analyzing the sequence of tokens it produces.  */
    bool Backtrack_Macro_Expansion(MacroExpansion *macroexp);

    /** Use the name of the macro as argument, as well as the expansion location.
        Macros can be redefined in weird ways such as:

        #define MAX(a, b) ((a) > (b) ? (a) : (b))
        #define A 3
        #define U MAX(A, 2)

        int f() {
          return U;
        }

        #define A 2

        int g() {
          return U;
        }

        One may think that functions f and g produce the same result, but actually
        they don't. Since macro A is redefined, the expansion of MAX(A, 2) will
        will have its value changed.  We take this into account by looking at the
        location where the macro is expanded. In `f` it will discard the last
        definition of A and get the last one above the expansion of U.  */
    bool Backtrack_Macro_Expansion(MacroInfo *info, const SourceLocation &loc);

    /** Iterate on the PreprocessingRecord through `it` until `loc` is reached.  */
    void Skip_Macros_Until(MacroIterator &it, const SourceLocation &loc);

    /** Iterate on the PreprocessingRecord through `it` until `loc` is reached,
        printing all macros reached in this path.  */
    void Print_Macros_Until(MacroIterator &it, const SourceLocation &loc, bool print=true);

    /** Iterate on the PreprocessingRecord through `it` until the end of the
        PreprocessingRecord is reached, printing all macros reached in this path.  */
    void Print_Remaining_Macros(MacroIterator &it);

    /** Return a new MacroIterator.  */
    MacroIterator Get_Macro_Iterator(void);

    /** Analyze macros in order to find references to constants declared in enums,
        like in enum { CONSTANT = 0 }; and then #define MACRO CONSTANT.  */
    void Include_Enum_Constants_Referenced_By_Macros(void);

    /* Populate the NeedsUndef vector whith macros that needs to be undefined
       somewhere in the code.  */
    int Populate_Need_Undef(void);

    /* Remove redundant decls, whose source location is inside another decl.  */
    void Remove_Redundant_Decls();


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

    /* Vector of MacroDirective of macros that needs to be undefined somewhere in
       the code.  */
    std::vector<MacroDirective*> NeedsUndef;

    MacroWalker MW;


    /** Check if a given declaration was already marked as dependency.  */
    inline bool Is_Decl_Marked(Decl *decl)
    { return Dependencies.find(decl) != Dependencies.end(); }

    /** Determine if a macro that are marked for output.  */
    inline bool Is_Macro_Marked(MacroInfo *x)
    { return x->isUsed(); }
};
