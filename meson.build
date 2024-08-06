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

# We need gcc at least 7.5.0
gcc = find_program('gcc', version : '>=7.5.0')

# We also need to get the system GCC installation folder to make sure we are
# building with the correct headers, otherwise we run the risk of building
# clang-extract with different headers than libLLVM
gcc_install_dir = ''
gcc_output = run_command(gcc.full_path(), '-v', check: true).stderr().strip().split('\n')
foreach line : gcc_output
  params = line.split('=')
  lhs = params.get(0)
  rhs = params.get(1, '')

  if lhs == 'COLLECT_LTO_WRAPPER'
    gcc_install_dir = run_command('dirname', rhs, check: true).stdout().strip()
    break
  endif
endforeach

# Check if we got the gcc installation dir and add it to the project arguments.
assert(gcc_install_dir != '', 'GCC headers dir not found. Check \'gcc -v\'')
add_project_arguments('--gcc-install-dir=' + gcc_install_dir, language: 'cpp')

########## Dependency: clang libraries ################
llvm_libdir = dependency('llvm', version : '>=16').get_variable(cmake : 'LLVM_LIBRARY_DIR', configtool: 'libdir')

clang_dep = []
clang_dep += cpp.find_library('clang-cpp', dirs : llvm_libdir)
clang_dep += cpp.find_library('LLVM', dirs : llvm_libdir)
############################# #########################
elf_dep = dependency('libelf') # libelf
zlib_dep = dependency('zlib')

subdir('libcextract')

incdir = include_directories('libcextract')

executable('ce-inline', 'Inline.cpp',
  include_directories : incdir,
  install : true,
  link_with : libcextract_static,
  dependencies : [elf_dep, zlib_dep]
)

executable('clang-extract', 'Main.cpp',
  include_directories : incdir,
  install : true,
  link_with : libcextract_static,
  dependencies : [elf_dep, clang_dep, zlib_dep]
)

#########
# Tests #
#########

subdir('testsuite')
