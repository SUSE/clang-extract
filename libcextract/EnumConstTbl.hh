#pragma once

#include <clang/Tooling/Tooling.h>
#include <unordered_map>

using namespace clang;

/** Maps EnumConstant to its respective EnumConstantDecl
 *
 * clang accept code like this:
 *
 * enum {
 *   const0 = 0,
 *   const1 = 1,
 * }
 *
 * int f(void) {
 *   return const1;
 * }
 *
 * In such cases, clang misses the reference to the enum and only stores that
 * there is a reference to a EnumConstantDecl, which is const1, and therefore
 * we lose the reference to the original enum.
 *
 * This class solves this problem by mapping the symbol to its original enum.
 *
*/

class EnumConstantTable
{
  public:

  /* Construct with empty table.  */
  EnumConstantTable(){}

  /* Construct adding all EnumDecl from ast.  */
  EnumConstantTable(ASTUnit *ast);

  bool Insert(EnumDecl *decl);

  EnumDecl *Get(const std::string &str);
  EnumDecl *Get(const StringRef &ref);
  EnumDecl *Get(const EnumConstantDecl *decl);

  void Dump(void);

  private:
  std::unordered_map<std::string, EnumDecl*> Table;
};
