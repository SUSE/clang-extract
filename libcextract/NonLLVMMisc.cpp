/* MISC functions that is used through the program but do not use any LLVM
   datastructure. This is put separately because we may want to build tools
   that do not use any LLVM datastructures, and because not including llvm
   headers compiles faster.  */

#include "NonLLVMMisc.hh"
#include <sys/types.h>
#include <sys/stat.h>

/** @brief Handle some quirks of getline.  */
char *getline_easy(FILE *file)
{
  ssize_t ret;
  size_t n = 0;
  char *line = nullptr;

  ret = getline(&line, &n, file);

  if (n == 0 || ret == -1) {
    free(line);
    return nullptr;
  }

  /* Remove any newline from the string.  */
  if (line[ret-1] == '\n')
    line[ret-1] = '\0';

  return line;
}

bool Is_Directory(const char *path)
{
  struct stat s;
  if (stat(path, &s)) {
    return false;
  } else {
    return S_ISDIR(s.st_mode);
  }
}

std::vector<std::string> Extract_Args(const char *str)
{
  std::vector<std::string> arg_list;

  const char *params = strchr(str, '=') + 1;
  char buf[strlen(params) + 1];
  const char *tok;

  strcpy(buf, params);
  tok = strtok(buf, ",");

  while (tok != nullptr) {
    arg_list.push_back(std::string(tok));
    tok = strtok(nullptr, ",");
  }

  return arg_list;
}

std::string Extract_Single_Arg(const char *str)
{
  const char *params = strchr(str, '=') + 1;
  char buf[strlen(params) + 1];
  const char *tok;

  strcpy(buf, params);
  tok = strtok(buf, ",");

  return std::string(tok);
}
