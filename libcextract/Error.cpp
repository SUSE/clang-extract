//===- Error.cpp - Implements a simple error pointing mechanism *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Implements a simple error pointing mechanism.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

#include "Error.hh"
#include "PrettyPrint.hh"
#include "NonLLVMMisc.hh"

DiagnosticOptionsWithColor::DiagnosticOptionsWithColor(void)
  : DOpts(new DiagnosticOptions())
{
  DOpts->ShowColors = check_color_available();
}

DiagsClass DiagsClass::sDiag;

DiagsClass::DiagsClass(void)
  : LangOpts(),
    DOpts(DiagnosticOptionsWithColor()),
    DiagsEngine(llvm::outs(), LangOpts, DOpts.getDiagsOptsToEngine())
{}

/* Print error giving a piece of source code that caused the error.  */
void DiagsClass::EmitMessage(const StringRef message, DiagnosticsEngine::Level level,
                             const SourceRange &range, const SourceManager &sm)
{
  CharSourceRange charsrc_range = CharSourceRange::getCharRange(range);
  FullSourceLoc loc = FullSourceLoc(range.getBegin(), sm);

  const std::string ce_message = Append_CE(message);
  DiagsEngine.emitDiagnostic(loc, level, ce_message, charsrc_range, FixItHint(), nullptr);
}
/* Print error without giving a piece of source code that caused the error.  */
void DiagsClass::EmitMessage(const StringRef message, DiagnosticsEngine::Level level)
{
  bool colored = Is_Colored();
  const std::string ce_message = Append_CE(message);
  TextDiagnostic::printDiagnosticLevel(llvm::outs(), level, colored);
  TextDiagnostic::printDiagnosticMessage(llvm::outs(), false, ce_message, 0, 0, colored);
}
