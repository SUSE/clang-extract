#include "FunctionExternalizeFinder.hh"
#include "FunctionDepsFinder.hh"

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

  if (externalized) {
    Externalize_Variables(dynamic_cast<FunctionDecl *>(decl));
  } else {
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
