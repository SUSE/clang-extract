#pragma once

#include <clang/Tooling/Tooling.h>
#include <unordered_set>
#include <unordered_map>

class FunctionDependencyFinder;

using namespace clang;

/** Shorthand to an iterator for the PreprocessingRecord.  */
typedef clang::PreprocessingRecord::iterator MacroIterator;

/** Macro Dependency Finder.
 *
 * This class wraps all code necessary to find all preprocessor directives
 * that are necessary to make the code compile.
 *
 * As an example, assume the following source.
 *
 * #define A 3
 * #define B 4
 *
 * int f(void) {
 *   return A;
 * }
 *
 * Here if we want to extract function A we must also output the macro `A`, and
 * discard the macro B. For that, the option
 * `-Xclang -detailed-preprocessing-record` must be passed do clang.
 *
 */
class MacroDependencyFinder
{
  public:

  /** Constructor which must take the Preprocessor used to parse the source
      file.  From an ASTUnit, one can provide the AST->getPreprocessor().  */
  MacroDependencyFinder(Preprocessor &p);

  /** Iterate on the PreprocessingRecord through `it` until `loc` is reached.  */
  void Skip_Macros_Until(MacroIterator &it, const SourceLocation &loc);

  /** Iterate on the PreprocessingRecord through `it` until `loc` is reached,
      printing all macros reached in this path.  */
  void Print_Macros_Until(MacroIterator &it, const SourceLocation &loc);

  /** Iterate on the PreprocessingRecord through `it` until the end of the
      PreprocessingRecord is reached, printing all macros reached in this path.  */
  void Print_Remaining_Macros(MacroIterator &it);

  /** Run the analysis to find which preprocessed directives are needed.  */
  void Find_Macros_Required(FunctionDependencyFinder *fdf, ASTUnit *ast);

  /** Return a new MacroIterator.  */
  inline MacroIterator Get_Macro_Iterator()
  { return PProcessor.getPreprocessingRecord()->begin(); };

  private:

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

  /** The object which holds the macro arguments is the MacroInfo.  Get the
      MacroInfo being extra careful to not pick the last definition, since
      macros can be redefined.  */
  MacroInfo *Get_Macro_Info(MacroDefinitionRecord *record);
  MacroInfo *Get_Macro_Info(MacroExpansion *macroexp);
  MacroInfo *Get_Macro_Info(const IdentifierInfo *id, const SourceLocation &loc);

  /** Preprocessor object used to parse the source file.  */
  Preprocessor &PProcessor;

  /* Hash containing the macros that are marked for output.  */
  std::unordered_set<MacroInfo*> Dependencies;

  /** Dump the Dependencies set for debug purposes.  */
  void Dump_Dependencies(void);

  /* Determine if a macro that are marked for output.  */
  inline bool Is_Macro_Marked(MacroInfo *x)
  { return Dependencies.find(x) != Dependencies.end(); }
};
