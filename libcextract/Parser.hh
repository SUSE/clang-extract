/**
 * Base class for parsers
 */

#pragma once

#include <vector>
#include <string>

class Parser
{
public:
  Parser(const std::string &path)
    : parser_path(path)
  {}

  Parser(const char *path)
    : parser_path(std::string(path))
  {}

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
  const std::string parser_path;
};
