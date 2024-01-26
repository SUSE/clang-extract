##===- meson.build - Rules for building clang-extract *- meson ----------*-===##
##
## This project is licensed under the Apache License v2.0 with LLVM Exceptions.
## See https://llvm.org/LICENSE.txt for license information.
## SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
##
##===----------------------------------------------------------------------===##
##
### \file
### Rules and scripts for building libcextract.
##
##===----------------------------------------------------------------------===##
#
# Author: Marcos Paulo de Souza

project('clang-extract', 'cpp',
  version : '0.1',
  default_options : [
    'cpp_std=c++20',
    'warning_level=3'
  ]
)

add_project_arguments(
  [
    '-Wno-unused-parameter',
    '-Wno-vla-extension'
  ], language: 'cpp'
)

# For debug builds, we want every debug info we can get plus addrsan.
buildtype = get_option('buildtype')
if buildtype == 'debug' or buildtype == 'debugoptimized'
  add_project_arguments('-g3', '-fsanitize=address', language : 'cpp')
  add_project_link_arguments('-fsanitize=address', language : 'cpp')
endif

# Ensure that we are using clang, and not gcc.
cpp = meson.get_compiler('cpp')
compiler_name = cpp.get_id()
# FIXME: We currently don't support g++, and get_id returns clang for clang++
assert(compiler_name == 'clang', compiler_name + ' found but only clang is supported')

########## Dependency: clang libraries ################
llvm_libdir = dependency('llvm', version : '>=16').get_variable(cmake : 'LLVM_LIBRARY_DIR', configtool: 'libdir')

clang_dep = []
clang_dep += cpp.find_library('clang-cpp', dirs : llvm_libdir)
clang_dep += cpp.find_library('LLVM', dirs : llvm_libdir)
############################# #########################
elf_dep = dependency('libelf') # libelf
python = import('python').find_installation('python3', modules : ['pexpect', 'psutil'])
find_program('gcc', version : '>=7.5.0')

subdir('libcextract')

incdir = include_directories('libcextract')

executable('inline', 'Inline.cpp',
  include_directories : incdir,
  link_with : libcextract_static,
  dependencies : elf_dep
)

executable('clang-extract', 'Main.cpp',
  include_directories : incdir,
  link_with : libcextract_static,
  dependencies : [elf_dep, clang_dep]
)

#########
# Tests #
#########

subdir('testsuite')
