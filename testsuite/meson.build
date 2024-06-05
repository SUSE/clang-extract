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
# Author: Giuliano Belinassi

runtest = find_program('lib/runtest.py')
ordinary_test_dirs = [ 'small/', 'includes/', 'ccp/', 'linux' ]

returncode_to_bool = [ true, false ]

foreach test_dir : ordinary_test_dirs
  subdir(test_dir) # Only here for meson to build the directory.
  test_files = run_command('find', test_dir, '-name', '*.c', '-o', '-name', '*.cpp',
                           check: true).stdout().strip().split('\n')

  foreach file : test_files
    test_path = meson.current_source_dir() + '/' + file
    outp_path = meson.current_build_dir() + '/' + file + '.log'
    should_fail = returncode_to_bool[
        run_command('grep', '-nwI', '{ *dg-xfail *}', test_path, check: false).returncode()]
    test(file, runtest, args : [ test_path, '-o', outp_path ], should_fail: should_fail)
  endforeach
endforeach

# Now the inline tests, which behaves differently.
subdir('inline')
