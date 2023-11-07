/**
 * Base class for parsers
 */

#pragma once

#include <vector>
#include <string>

class Parser
{
public:
  Parser(const char *path)
    : parser_path(path)
  {}

  // Some parsers work on files
  void Parse();
  void Parse(const char *path);

  std::vector<std::string> Get_All_Symbols();
  bool Needs_Externalization(const std::string &symbol);

protected:
  const char *parser_path;
};
