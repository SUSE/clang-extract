#include "FunctionExternalizeFinder.hh"
#include "FunctionDepsFinder.hh"

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
static CallGraph *Build_CallGraph_From_AST(ASTUnit *ast)
{
  CallGraph *cg = new CallGraph();

  /* Iterate trough all nodes in toplevel.  */
  clang::ASTUnit::top_level_iterator it;
  for (it = ast->top_level_begin(); it != ast->top_level_end(); ++it) {
    Decl *decl = *it;

    /* Add decl node to the callgraph.  */
    cg->addToCallGraph(decl);
  }

  return cg;
}

FunctionExternalizeFinder::FunctionExternalizeFinder(ASTUnit *ast, std::vector<std::string> &to_extract, std::vector<std::string> &to_externalize)
  : AST(ast)
{
  for (const std::string &name : to_extract) {
    MustNotExternalize.insert(name);
  }

  for (const std::string &name : to_externalize) {
    MustExternalize.insert(name);
  }

  Run_Analysis();

  /* Compute intersection.  They must be empty.  */
  std::unordered_set<std::string> c;
  for (const std::string &e : MustNotExternalize) {
    if (MustExternalize.count(e) > 0) {
      c.insert(e);
    }
  }

  assert(c.size() == 0 && "Exernalize INTERSECTION Export is not empty.");
}

bool FunctionExternalizeFinder::Should_Externalize(CallGraphNode *node)
{
  return Should_Externalize(dynamic_cast<FunctionDecl *>(node->getDecl()));
}

bool FunctionExternalizeFinder::Should_Externalize(FunctionDecl *decl)
{
  if (!decl)
    return false;

  return true;
}

bool FunctionExternalizeFinder::Analyze_Node(CallGraphNode *node)
{
  bool externalized = false;
  NamedDecl *decl = dynamic_cast<NamedDecl *>(node->getDecl());

  const std::string &name = decl->getName().str();

  if (!Must_Not_Externalize(name)) {
    externalized = Mark_For_Externalization(name);
  }

  if (!externalized) {
    Externalize_Variables(dynamic_cast<FunctionDecl *>(decl));
    CallGraphNode::const_iterator child_it;
    for (child_it = node->begin(); child_it != node->end(); ++child_it) {
      CallGraphNode *child = child_it->Callee;

      if (Should_Externalize(child)) {
        Analyze_Node(child);
      }
    }
  }

  return externalized;
}

bool FunctionExternalizeFinder::Externalize_Variables(FunctionDecl *decl)
{
  if (!decl)
    return false;

  bool externalized = false;

  Stmt *body = decl->getBody();
  if (body) {
    externalized = Externalize_Variables(body);
  }

  return externalized;
}

bool FunctionExternalizeFinder::Externalize_Variables(Stmt *stmt)
{
  if (!stmt)
    return false;

  bool externalized = false;

  if (DeclRefExpr::classof(stmt)) {
    DeclRefExpr *expr = (DeclRefExpr *) stmt;
    VarDecl *decl = dynamic_cast<VarDecl *>(expr->getDecl());

    if (decl && decl->hasGlobalStorage()) {
      externalized = Mark_For_Externalization(decl->getName().str());
    }
  }

  /* Iterate through the list of all children Statements and repeat the
     process.  */
  clang::Stmt::child_iterator it, it_end;
  for (it = stmt->child_begin(), it_end = stmt->child_end();
      it != it_end; ++it) {

    Stmt *child = *it;
    externalized |= Externalize_Variables(child);
  }

  return externalized;
}

void FunctionExternalizeFinder::Run_Analysis(void)
{
  CallGraph *cg = Build_CallGraph_From_AST(AST);
  CallGraph::const_iterator it;

  for (it = cg->begin(); it != cg->end(); ++it) {
    CallGraphNode *node = (*it).getSecond().get();
    if (Should_Externalize(node)) {
      Analyze_Node(node);
    }
  }

  delete cg;
}

std::vector<std::string> FunctionExternalizeFinder::Get_To_Externalize(void)
{
  std::vector<std::string> externalize;

  for (const std::string &name : MustExternalize) {
    externalize.push_back(name);
  }

  return externalize;
}
