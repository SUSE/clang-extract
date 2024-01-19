//===- Error.hh - Implements a simple error pointing mechanism *- C++ ---*-===//
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

#pragma once

#include <clang/Frontend/TextDiagnostic.h>
#include <clang/Basic/LangOptions.h>

using namespace clang;

/* Creates a special DiagnosticOptions with forced ShowColors.  */
class DiagnosticOptionsWithColor
{
  public:
  DiagnosticOptionsWithColor(void);

  inline DiagnosticOptions *Get_DiagnosticOptions(void)
  {
    return DOpts;
  }

  inline bool Is_Colored(void)
  {
    return DOpts->ShowColors;
  }

  private:
  DiagnosticOptions *DOpts;
};

/* Diagnostics class, used to wrap error and warning messages.  */
class DiagsClass
{
  public:
  DiagsClass(void);

  void EmitMessage(const StringRef message, DiagnosticsEngine::Level level,
                   const SourceRange &range);

  void EmitMessage(const StringRef message, DiagnosticsEngine::Level level);

  private:
  inline const std::string Append_CE(StringRef message)
  {
    return "clang-extract: " + message.str();
  }

  LangOptions LangOpts;
  DiagnosticOptionsWithColor DOpts;
  TextDiagnostic DiagsEngine;

  protected:
  static DiagsClass sDiag;

  public:
  DiagsClass(DiagsClass &o) = delete;
  void operator=(const DiagsClass &o) = delete;

  static inline DiagsClass &Get_Instance(void)
  {
    return sDiag;
  }

  static inline bool Is_Colored(void)
  {
    return Get_Instance().DOpts.Is_Colored();
  }

  static inline void Emit_Message(const StringRef message,
                                  DiagnosticsEngine::Level level,
                                  const SourceRange &range)
  {
    sDiag.EmitMessage(message, level, range);
  }

  static inline void Emit_Message(const StringRef message,
                                  DiagnosticsEngine::Level level)
  {
    sDiag.EmitMessage(message, level);
  }

  static inline void Emit_Error(const StringRef message, const SourceRange &range)
  {
    Emit_Message(message, DiagnosticsEngine::Level::Error, range);
  }

  static inline void Emit_Error(const StringRef message)
  {
    Emit_Message(message, DiagnosticsEngine::Level::Error);
  }

  static inline void Emit_Warn(const StringRef message, const SourceRange &range)
  {
    Emit_Message(message, DiagnosticsEngine::Level::Warning, range);
  }

  static inline void Emit_Warn(const StringRef message)
  {
    Emit_Message(message, DiagnosticsEngine::Level::Warning);
  }

  static inline void Emit_Note(const StringRef message, const SourceRange &range)
  {
    Emit_Message(message, DiagnosticsEngine::Level::Note, range);
  }

  static inline void Emit_Note(const StringRef message)
  {
    Emit_Message(message, DiagnosticsEngine::Level::Note);
  }
};
