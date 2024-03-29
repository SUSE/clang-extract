//===- Parser.hh - Base class for parsers -----------------------*- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Base class for parsers.
//
//===----------------------------------------------------------------------===//

/* Author: Marcos Paulo de Souza  */

#pragma once

#include <vector>
#include <string>
#include <string.h>
#include <errno.h>
#include <stdexcept>
#include <wordexp.h>

class Parser
{
public:
  Parser(const std::string &path)
    : Parser(path.c_str())
  {
  }

  Parser(const char *path)
  {
    /* Expand shell characters like '~' in paths.  */
    wordexp_t exp_result;
    wordexp(path, &exp_result, 0);

    /* Store in parser_path.  */
    parser_path = std::string(exp_result.we_wordv[0]);

    /* Release expansion object.  */
    wordfree(&exp_result);
  }

  // Some parsers work on files
  void Parse();
  void Parse(const char *path);

  std::vector<std::string> Get_All_Symbols();
  bool Needs_Externalization(const std::string &symbol);
  inline const std::string &Get_Path(void)
  {
    return parser_path;
  }


protected:
  std::string parser_path;
};
