#include "EnumConstTbl.hh"
#include <iostream>

EnumConstantTable::EnumConstantTable(ASTUnit *ast)
{
  clang::ASTUnit::top_level_iterator it;
  for (it = ast->top_level_begin(); it != ast->top_level_end(); ++it) {
    EnumDecl *decl = dynamic_cast<EnumDecl *>(*it);

    if (decl && decl->isThisDeclarationADefinition()) {
      Insert(decl);
    }
  }
}

bool EnumConstantTable::Insert(EnumDecl *decl)
{
  bool inserted = false;

  for (EnumConstantDecl *field : decl->enumerators()) {
    const std::string symbol = field->getNameAsString();

    /* Ensure that such symbol was not inserted before.  */
    assert(!Get(symbol));

    /* Insert symbol in table.  */
    Table[symbol] = decl;
  }

  return inserted;
}

EnumDecl *EnumConstantTable::Get(const std::string &str)
{
  return Table[str];
}

EnumDecl *EnumConstantTable::Get(const StringRef &ref)
{
  return Get(ref.str());
}

EnumDecl *EnumConstantTable::Get(const EnumConstantDecl *ecdecl)
{
  return Get(ecdecl->getName());
}

void EnumConstantTable::Dump(void)
{
  for (auto &p: Table)
    std::cout << " " << p.first << " => " << p.second << '\n';
}
