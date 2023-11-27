#include "DscFileGenerator.hh"
#include "PrettyPrint.hh"
#include "clang/AST/DeclContextInternals.h"
#include "Error.hh"

#include <stdexcept>

/** Get a Decl object from its Identifier by looking into a table (O(1)).  */
static NamedDecl *Get_Decl_From_Identifier(ASTUnit *ast, const IdentifierInfo &info)
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

  return x.getAsDecl();
}

/** Get a Decl object from its Identifier by looking into a table (O(1)).  */
static NamedDecl *Get_Decl_From_Identifier(ASTUnit *ast, const StringRef name)
{
  IdentifierTable &IdTbl = ast->getPreprocessor().getIdentifierTable();
  return Get_Decl_From_Identifier(ast, IdTbl.get(name));
}

DscFileGenerator::DscFileGenerator(
      const char *output,
      ASTUnit *ast,
      const std::vector<std::string> &funcs_to_extract,
      const std::vector<ExternalizerLogEntry> &log,
      InlineAnalysis &ia)
  : OutputPath(output),
    Out(OutputPath, EC),
    AST(ast),
    FuncsToExtract(funcs_to_extract),
    ExternalizerLog(log),
    IA(ia)
{
  std::error_condition ok;
  if (EC != ok) {
    throw std::runtime_error("unable to open file " + OutputPath + " for writing: " + EC.message());
  }

  Run_Analysis();
}

void DscFileGenerator::Target_ELF(void)
{
  if (IA.Have_Debuginfo()) {
    Out << "LIVEPATH_CONTAINER"
        << "\n@" << IA.Get_Debuginfo_Path();
  } else {
    Out << "LIVEPATH_CONTAINER"
        << "\n@LIVEPATCH_TARGET";

    DiagsClass::Emit_Warn("No target ELF given.  Generated libpulp .dsc is incomplete.");
  }
}

void DscFileGenerator::Global_Functions(void)
{
  bool have_renamed_symbols = false;
  for (const ExternalizerLogEntry &entry : ExternalizerLog) {
    if (entry.Type == ExternalizationType::RENAME) {
      have_renamed_symbols = true;
      break;
    }
  }

  if (have_renamed_symbols) {
    for (const ExternalizerLogEntry &entry : ExternalizerLog) {
      if (entry.Type == ExternalizationType::RENAME) {
        NamedDecl *decl = Get_Decl_From_Identifier(AST, entry.NewName);
        if (decl == nullptr) {
          throw std::runtime_error("Unable to find symbol " + entry.NewName
                                                            + " in the AST");
        }
        Out << '\n' << entry.OldName << ":" << entry.NewName;
      }
    }
  } else {
    for (const std::string &func_name : FuncsToExtract) {
      NamedDecl *decl = Get_Decl_From_Identifier(AST, func_name);
      if (decl == nullptr) {
        throw std::runtime_error("Unable to find symbol " + func_name + " in the AST");
      }
      Out << '\n' << func_name << ":" << func_name;
    }
  }
}

void DscFileGenerator::Local_Symbols(void)
{
  for (const ExternalizerLogEntry &entry : ExternalizerLog) {
    if (entry.Type == ExternalizationType::STRONG) {
      NamedDecl *decl = Get_Decl_From_Identifier(AST, entry.NewName);
      if (decl == nullptr) {
        throw std::runtime_error("Unable to find symbol " + entry.NewName + " in the AST");
      }
      Out << "\n#" << entry.OldName << ":" << entry.NewName;
      std::string mod = IA.Get_Symbol_Module(entry.OldName);
      if (!mod.empty())
        Out << ":" << mod;
    }
  }
}

void DscFileGenerator::Run_Analysis(void)
{
  Target_ELF();
  Global_Functions();
  Local_Symbols();
}
