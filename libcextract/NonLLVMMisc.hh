/* MISC functions that is used through the program but do not use any LLVM
   datastructure. This is put separately because we may want to build tools
   that do not use any LLVM datastructures, and because not including llvm
   headers compiles faster.  */

#pragma once

#include <stdbool.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>

/** Stringfy constant integer token in `s`.  */
#define STRINGFY_VALUE(s) STRINGFY(s)

/** Stringfy given expression `s`.  */
#define STRINGFY(s) #s

/** Handle `free(x)`, where x is not a void* pointer.  */
#define free(x) free((void*)(x))

/** Get number of element of array declared statically.  */
#define ARRAY_LENGTH(v) (sizeof(v)/sizeof(*v))

/* Check if string a is a prefix of string b.  */
inline bool prefix(const char *a, const char *b)
{
  return !strncmp(a, b, strlen(a));
}

/** Check if a string is null or empty.  */
inline bool is_null_or_empty(const char *str)
{
  return str == nullptr || *str == '\0';
}

template <typename T>
void Remove_Duplicates(std::vector<T>& vec)
{
  std::sort(vec.begin(), vec.end());
  vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}

/** Get a single line from a file, removing its newline.
  *
  * NOTE: if this function return a valid pointer, it must be free'd.
  */
char *getline_easy(FILE *file);

/** Check if given path is a directory.  */
bool Is_Directory(const char *path);

/** Extract arguments that are specified after the '=' sign separated by ','.  */
std::vector<std::string> Extract_Args(const char *str);

/** Extract argument that are specified after the '=' sign.  */
std::string Extract_Single_Arg(const char *str);

/** Extract argument that are specified after the '=' sign.  */
const char *Extract_Single_Arg_C(const char *str);

/** Check if output supports colors.  */
bool check_color_available(void);
