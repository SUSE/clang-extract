#pragma once

#include <clang/Tooling/Tooling.h>
#include <unordered_set>

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
  void Find_Macros_Required(ASTUnit *ast);

  /** Return a new MacroIterator.  */
  inline MacroIterator Get_Macro_Iterator()
  { return PProcessor.getPreprocessingRecord()->begin(); };

  private:

  /** Preprocessor object used to parse the source file.  */
  Preprocessor &PProcessor;

  /* Hash containing the macros that are marked for output.  */
  std::unordered_set<MacroDefinitionRecord*> Dependencies;

  /* Determine if a macro that are marked for output.  */
  inline bool Is_Macro_Marked(MacroDefinitionRecord *prerec)
  { return Dependencies.find(prerec) != Dependencies.end(); }
};
