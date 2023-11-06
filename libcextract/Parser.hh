/**
 * Base class for parsers
 */

#pragma once

class Parser
{
public:
  Parser(const char *path)
    : parser_path(path)
  {}

  // Some parsers work on files
  void Parse();
  void Parse(const char *path);

protected:
  const char *parser_path;
};
