#include "Error.hh"
#include "PrettyPrint.hh"

DiagsClass DiagsClass::sDiag;

DiagsClass::DiagsClass(void)
  : LangOpts(),
    DOpts(DiagnosticOptionsWithColor()),
    DiagsEngine(llvm::outs(), LangOpts, DOpts.Get_DiagnosticOptions())
{}

void DiagsClass::EmitMessage(const StringRef message, DiagnosticsEngine::Level level, const SourceRange &range)
{
  SourceManager *sm = PrettyPrint::Get_Source_Manager();
  if (sm) {
    CharSourceRange charsrc_range = CharSourceRange::getCharRange(range);
    FullSourceLoc loc = FullSourceLoc(range.getBegin(), *sm);

    const std::string ce_message = Append_CE(message);
    DiagsEngine.emitDiagnostic(loc, level, ce_message, charsrc_range, FixItHint(), nullptr);
  } else {
    EmitMessage(message, level);
  }
}

void DiagsClass::EmitMessage(const StringRef message, DiagnosticsEngine::Level level)
{
  const std::string ce_message = Append_CE(message);
  TextDiagnostic::printDiagnosticLevel(llvm::outs(), level, true);
  TextDiagnostic::printDiagnosticMessage(llvm::outs(), false, ce_message, 0, 0, true);
}
