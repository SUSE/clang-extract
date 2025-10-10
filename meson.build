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

fs = import('fs')

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
    if build_machine.system() == 'linux'
      #Check whether the found folder good enough in Linux
      if fs.is_file(gcc_install_dir / 'crtbegin.o')
        break
      endif
    else
      break
    endif
    # If not good enough we need to try harder
    if not fs.is_file(gcc_install_dir / 'crtbegin.o') and build_machine.system() == 'linux'
      sh = find_program('sh')
      gcc_path = fs.parent(fs.parent(gcc.full_path()))
      print_str = '\'{print $1}\''
      find_str_64 = 'find ' + gcc_path + ' -name crtbegin.o -type f -exec file {} \; | grep "ELF 64-bit" | awk -F: ' + print_str + ' | xargs dirname | head -1'
      gcc_install_dir = run_command(sh.full_path(),'-c', find_str_64, check: false).stdout().strip().split('\n')[0]
      if not fs.is_file(gcc_install_dir / 'crtbegin.o')
        find_str = 'find ' + gcc_path + ' -name crtbegin.o -type f -exec file {} \; | awk -F: ' + print_str + ' | xargs dirname | head -1'
        gcc_install_dir = run_command(sh.full_path(), '-c', find_str).stdout().strip().split('\n')[0]
      endif
    endif
    break
  endif
endforeach

# Check if we got the gcc installation dir and add it to the project arguments.
assert(gcc_install_dir != '', 'GCC headers dir not found. Check \'gcc -v\'')
add_project_arguments('--gcc-install-dir=' + gcc_install_dir, language: 'cpp')

########## Dependency: additional clang includes ################
llvm_config = find_program('llvm-config')
if llvm_config.found()
  extra_args_line = run_command(llvm_config.full_path(), '--cxxflags').stdout().strip().split('\n')[0]
  extra_args = extra_args_line.split(' -')
  i = 0
  foreach arg : extra_args
    targ = arg
    if i > 0
      targ = '-' + arg
    endif
    if(not targ.startswith('-I'))
      i += 1
      continue
    endif
    message('Adding extra compilation flag: ' + targ)
    add_project_arguments(targ, language: 'cpp')
    i += 1
  endforeach
endif

########## Dependency: clang libraries ################
llvm_libdir = dependency('llvm', version : '>=16').get_variable(cmake : 'LLVM_LIBRARY_DIR', configtool: 'libdir')

clang_dep = []
clang_dep += cpp.find_library('clang-cpp', dirs : llvm_libdir)
clang_dep += cpp.find_library('LLVM', dirs : llvm_libdir)
############################# #########################
elf_dep = dependency('libelf') # libelf
zlib_dep = dependency('zlib')
zstd_dep = dependency('libzstd')

subdir('libcextract')

incdir = include_directories('libcextract')

executable('ce-inline', 'Inline.cpp',
  include_directories : incdir,
  install : true,
  link_args : ['--gcc-install-dir=' + gcc_install_dir],
  link_with : libcextract_static,
  dependencies : [elf_dep, zlib_dep, zstd_dep]
)

executable('clang-extract', 'Main.cpp',
  include_directories : incdir,
  install : true,
  link_args : ['--gcc-install-dir=' + gcc_install_dir],
  link_with : libcextract_static,
  dependencies : [elf_dep, clang_dep, zlib_dep, zstd_dep]
)

#########
# Tests #
#########

subdir('testsuite')
