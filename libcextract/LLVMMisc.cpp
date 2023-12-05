/* MISC functions that is used through the program and use any LLVM
   datastructure.  */

#include "LLVMMisc.hh"

/** Check if Decl is a builtin.  */
bool Is_Builtin_Decl(const Decl *decl)
{
  const NamedDecl *ndecl = dyn_cast<const NamedDecl>(decl);
  if (ndecl) {
    /* Avoid triggering an assert in clang.  */
    if (ndecl->getDeclName().isIdentifier()) {
      StringRef name = ndecl->getName();
      if (name.starts_with("__builtin_")) {
        return true;
      }

      if (name.starts_with("__compiletime_assert_")) {
        return true;
      }
    }
  }

  return false;
}

/** Get a Decl object from its Identifier by looking into a table (O(1)).  */
const StoredDeclsList &Get_Stored_Decl_List_From_Identifier(ASTUnit *ast,
                                                            const IdentifierInfo &info)
{
  TranslationUnitDecl *tu = ast->getASTContext().getTranslationUnitDecl();
  StoredDeclsMap *map = tu->getLookupPtr();
  if (map == nullptr) {
    /* We need to build it.  */
    map = tu->buildLookup();
  }
  assert(map && "Lookup map is null.");

  DeclarationName dn(&info);
  auto found = map->find(dn);
  if (found == map->end()) {
    /* Well, throw to whoever called me.  */
    throw SymbolNotFoundException(info.getName().str());
  }

  const StoredDeclsList &x = found->second;
  return x;
}

NamedDecl *Get_Decl_From_Identifier(ASTUnit *ast, const IdentifierInfo &info)
{
  return Get_Stored_Decl_List_From_Identifier(ast, info).getAsDecl();
}

/** Get a Decl object from its Identifier by looking into a table (O(1)).  */
NamedDecl *Get_Decl_From_Identifier(ASTUnit *ast, const StringRef &name)
{
  IdentifierTable &IdTbl = ast->getPreprocessor().getIdentifierTable();
  return Get_Decl_From_Identifier(ast, IdTbl.get(name));
}

const DeclContextLookupResult Get_Decl_List_From_Identifier(ASTUnit *ast,
                                                            const IdentifierInfo &info)
{
  const StoredDeclsList &list = Get_Stored_Decl_List_From_Identifier(ast, info);
  return list.getLookupResult();
}

const DeclContextLookupResult Get_Decl_List_From_Identifier(ASTUnit *ast,
                                                            const StringRef &name)
{
  IdentifierTable &IdTbl = ast->getPreprocessor().getIdentifierTable();
  return Get_Decl_List_From_Identifier(ast, IdTbl.get(name));
}

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
CallGraph *Build_CallGraph_From_AST(ASTUnit *ast)
{
  CallGraph *cg = new CallGraph();
  cg->TraverseAST(ast->getASTContext());

  return cg;
}


FunctionDecl *Get_Bodyless_Decl(FunctionDecl *decl)
{
  while (decl) {
    if (decl->hasBody()) {
      decl = decl->getPreviousDecl();
    } else {
      return decl;
    }
  }

  return nullptr;
}

FunctionDecl *Get_Bodyless_Or_Itself(FunctionDecl *decl)
{
  FunctionDecl *bodyless = Get_Bodyless_Decl(decl);
  return bodyless ? bodyless : decl;
}

TagDecl *Get_Bodyless_Decl(TagDecl *decl)
{
  while (decl) {
    if (decl->isCompleteDefinition()) {
      decl = decl->getPreviousDecl();
    } else {
      return decl;
    }
  }

  return nullptr;
}

TagDecl *Get_Bodyless_Or_Itself(TagDecl *decl)
{
  TagDecl *bodyless = Get_Bodyless_Decl(decl);
  return bodyless ? bodyless : decl;
}

Decl *Get_Bodyless_Decl(Decl *decl)
{
  if (TagDecl *tag = dyn_cast<TagDecl>(decl)) {
    return Get_Bodyless_Decl(tag);
  } else if (FunctionDecl *func = dyn_cast<FunctionDecl>(decl)) {
    return Get_Bodyless_Decl(func);
  }

  return nullptr;
}

Decl *Get_Bodyless_Or_Itself(Decl *decl)
{
  Decl *bodyless = Get_Bodyless_Decl(decl);
  return bodyless ? bodyless : decl;
}

