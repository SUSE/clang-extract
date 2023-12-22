#include "InlineAnalysis.hh"

#include "clang/Analysis/CallGraph.h"
#include "clang/Frontend/ASTUnit.h"
#include "InlineAnalysis.hh"

#include <vector>
#include <unordered_set>

using namespace clang;

class FunctionExternalizeFinder
{
  public:
  FunctionExternalizeFinder(ASTUnit *ast,
                            std::vector<std::string> &to_extract,
                            std::vector<std::string> &to_exernalize,
                            bool keep_includes,
                            InlineAnalysis &IA);

  bool Should_Externalize(CallGraphNode *node);
  bool Should_Externalize(const DeclaratorDecl *decl);

  void Run_Analysis(void);

  std::vector<std::string> Get_To_Externalize(void);

  protected:

  bool Analyze_Node(CallGraphNode *);
  bool Analyze_Function(FunctionDecl *);

  bool Externalize_DeclRefs(FunctionDecl *decl);
  bool Externalize_DeclRefs(Stmt *stmt);

  inline bool Must_Not_Externalize(const std::string &name)
  {
    return MustNotExternalize.find(name) != MustNotExternalize.end();
  }

  inline bool Must_Not_Externalize(const DeclaratorDecl *decl)
  {
    return Must_Not_Externalize(decl->getName().str());
  }

  inline bool Is_Marked_For_Externalization(const std::string &name)
  {
    return MustExternalize.find(name) != MustExternalize.end();
  }

  inline bool Mark_For_Externalization(const std::string &name)
  {
    bool externalized = false;

    if (!Is_Marked_For_Externalization(name)) {
      externalized = true;
      MustExternalize.insert(name);
    }

    return externalized;
  }

  std::unordered_set<std::string> MustExternalize;
  std::unordered_set<std::string> MustNotExternalize;
  std::unordered_set<std::string> ToExtract;

  inline bool Should_Extract(const std::string &name)
  {
    return ToExtract.find(name) != ToExtract.end();
  }

  inline bool Should_Extract(const FunctionDecl *func)
  {
    return Should_Extract(func->getName().str());
  }

  std::unordered_set<const CallGraphNode *> AnalyzedNodes;
  bool Is_Already_Analyzed(const CallGraphNode* node)
  {
    return AnalyzedNodes.find(node) != AnalyzedNodes.end();
  }

  bool KeepIncludes;
  ASTUnit *AST;
  InlineAnalysis &IA;
};
