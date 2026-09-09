//===- NonLLVMMisc.hh - Implement misc functions that don´t depends on LLVM *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// MISC functions that is used through the program and do not use any LLVM
/// datastructure.
//
//===----------------------------------------------------------------------===//

/* Author: Giuliano Belinassi  */

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

/** Check if suffix is a suffix of s.  */
inline bool suffix(const char *suffix, const char *s)
{
  size_t n = strlen(s);
  size_t m = strlen(suffix);
  return n >= m && strcmp(s + n - m, suffix) == 0;
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

template <typename T>
bool Remove_Elements_Present_In_2nd_Vector(std::vector<T>& v1, const std::vector<T> &v2)
{
  bool ret = false;

  if (v2.size() == 0)
    return false;

  /* Remove any element in to_externalize that is in NotExternalize.  */
  for (auto it = v1.begin(); it != v1.end(); it++) {
    for (auto ij = v2.begin(); ij != v2.end(); ij++) {
      if (*it == *ij) {
        v1.erase(it);
        it--;
        ret = true;
      }
    }
  }

  return ret;
}

/** Get a single line from a file, removing its newline.
  *
  * NOTE: if this function return a valid pointer, it must be free'd.
  */
char *getline_easy(FILE *file);

/** Check if given path is a directory.  */
bool Is_Directory(const char *path);

/** Simulates the behaviour of `mkdir -p`.  That means, if you pass a path to
    `path`, it will create a chain of directories to it.  */
int mkdir_p(const char *path, mode_t mode);

int Ensure_Path_Exists(const std::string &);

/** Extract arguments that are specified after the '=' sign separated by ','.  */
std::vector<std::string> Extract_Args(const char *str);

/** Extract argument that are specified after the '=' sign.  */
std::string Extract_Single_Arg(const char *str);

/** Extract argument that are specified after the '=' sign.  */
const char *Extract_Single_Arg_C(const char *str);

/** Check if output supports colors.  */
bool check_color_available(void);

class FileHandling
{
  public:
  enum FileType {
    FILE_TYPE_ELF,
    FILE_TYPE_GZ,
    FILE_TYPE_ZSTD,
    FILE_TYPE_UNKNOWN,
  };

  static enum FileType Get_File_Type(int fd);
};

/** Get basename of a string.  Works like the gnu version.  */
const char *get_basename(const char *filename);

/** Get the relative path from aboslute path.  */
std::string Get_Relative_Path(const std::string &path);
