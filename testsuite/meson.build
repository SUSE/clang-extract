runtest = find_program('lib/runtest.py')
ordinary_test_dirs = [ 'small/', 'includes/', 'ccp/' ]

returncode_to_bool = [ true, false ]

foreach test_dir : ordinary_test_dirs
  subdir(test_dir) # Only here for meson to build the directory.
  grabber = 'find ' + test_dir + ' -name *.c -o -name *.cpp'
  test_files = run_command('sh', '-c', grabber, check: true).stdout().strip().split('\n')

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
