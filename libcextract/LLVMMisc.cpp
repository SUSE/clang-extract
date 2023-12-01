/* MISC functions that is used through the program and use any LLVM
   datastructure.  */

#include "LLVMMisc.hh"

/** Check if Decl is a builtin.  */
bool Is_Builtin_Decl(const Decl *decl)
{
  const NamedDecl *ndecl = dyn_cast<const NamedDecl>(decl);
  if (ndecl) {
    StringRef name = ndecl->getName();
    if (name.starts_with("__builtin_")) {
      return true;
    }

    if (name.starts_with("__compiletime_assert_")) {
      return true;
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

