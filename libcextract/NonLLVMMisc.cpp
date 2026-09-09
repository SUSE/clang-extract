//===- NonLLVMMisc.cpp - Implement misc functions that don´t depends on LLVM *- C++ -*-===//
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

#include "NonLLVMMisc.hh"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <assert.h>

#include <filesystem>
#include <iostream>

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

/** Simulates the behaviour of `mkdir -p`.  That means, if you pass a path to
    `path`, it will create a chain of directories to it.  */
int mkdir_p(const char *path, mode_t mode)
{
    char tmp[PATH_MAX];
    size_t len;

    if (path == NULL || *path == '\0') {
        errno = EINVAL;
        return -1;
    }

    len = strlen(path);

    if (len >= sizeof(tmp)) {
        errno = ENAMETOOLONG;
        return -1;
    }

    memcpy(tmp, path, len + 1);

    /* Remove trailing slashes, except for "/" itself. */
    while (len > 1 && tmp[len - 1] == '/') {
        tmp[--len] = '\0';
    }

    /*
     * Walk through the path:
     *
     *   /a/b/c
     *   ^   ^
     *   |   |
     *   mkdir /a
     *       mkdir /a/b
     *           mkdir /a/b/c
     */
    for (char *p = tmp + 1; *p != '\0'; ++p) {
        if (*p != '/')
            continue;

        *p = '\0';

        if (mkdir(tmp, mode) == -1 && errno != EEXIST) {
            return -1;
        }

        *p = '/';
    }

    /* Create the final component. */
    if (mkdir(tmp, mode) == -1 && errno != EEXIST) {
        return -1;
    }

    /*
     * EEXIST isn't enough: the existing object could be a regular
     * file rather than a directory.
     */
    struct stat st;

    if (stat(tmp, &st) == -1)
        return -1;

    if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return -1;
    }

    return 0;
}

int Ensure_Path_Exists(const std::string &path)
{
  char buffer[PATH_MAX] = "";
  int m = path.length();

  if (m >= PATH_MAX) {
    return ENAMETOOLONG;
  }

  memcpy(buffer, path.c_str(), m);
  char *slash = strrchr(buffer, '/');
  if (slash) {
    *slash = '\0';
    return mkdir_p(buffer, 0755);
  }
  /* It should be the current path, so its fine.  */
  return 0;
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
  const char *params = Extract_Single_Arg_C(str);
  char buf[strlen(params) + 1];
  const char *tok;

  strcpy(buf, params);
  tok = strtok(buf, ",");

  return std::string(tok);
}

const char *Extract_Single_Arg_C(const char *str)
{
  const char *params = strchr(str, '=') + 1;
  return params;
}

bool check_color_available(void)
{
  /* Check if NO_COLOR env variable is set.  */
  if (getenv("NO_COLOR"))
    return false;

  const char *term = getenv("TERM");
  return term && strcmp(term, "dumb") != 0 && isatty(STDOUT_FILENO);
}

enum FileHandling::FileType FileHandling::Get_File_Type(int fd)
{

  unsigned char buf[4] = { 0 };

  /* Make sure that we are in the beginning of the file */
  lseek(fd, 0, SEEK_SET);

  int ret = read(fd, buf, 4);
  if (ret < 0) {
   throw std::runtime_error("error reading ELF file: " + std::to_string(ret));
  } else if (ret == 0) {
   throw std::runtime_error("error reading ELF file: empty file?");
  }

  /* reset file pointer */
  lseek(fd, 0, SEEK_SET);

  const unsigned char elf_magic[4] =  { 0x7f, 0x45, 0x4c, 0x46 };
  const unsigned char zlib_magic[2] = { 0x1f, 0x8b };
  const unsigned char zstd_magic[4] = { 0x28, 0xb5, 0x2f, 0xfd };

  /* Check if the file is an ELF */
  if (!memcmp(buf, elf_magic, 4))
    return FILE_TYPE_ELF;

  else if (!memcmp(buf, zlib_magic, 2))
    return FILE_TYPE_GZ;

  else if (!memcmp(buf, zstd_magic, 4))
    return FILE_TYPE_ZSTD;

  return FILE_TYPE_UNKNOWN;
}

/** Get basename of a string.  Works like the gnu version.  */
const char *get_basename(const char *path)
{
  const char *base = strrchr(path, '/');
#ifdef _WIN32
  const char *backslash = strrchr(path, '\\');
  if (backslash > base)
    base = backslash;
#endif
  return base ? base+1 : path;
}

/** Get the relative path from aboslute path.  */
std::string Get_Relative_Path(const std::string &path)
{
  char cwd[PATH_MAX];
  assert(getcwd(cwd, sizeof(cwd)) != nullptr);

  return std::filesystem::relative(path, cwd);
}
