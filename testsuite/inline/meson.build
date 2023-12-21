grabber = 'for i in *.c; do echo $i; done'
test_files = run_command('sh', '-c', grabber, check: true).stdout().strip().split('\n')

foreach file : test_files
  test_path = meson.current_source_dir() + '/' + file
  outp_path = meson.current_build_dir() + '/' + file + '.log'

  should_fail = returncode_to_bool[
      run_command('grep', '-nwI', '{ *dg-xfail *}', test_path, check: false).returncode()]
  test(file, runtest, args : [ '-inline-test', test_path, '-o', outp_path ],
             should_fail: should_fail)
endforeach

subdir('lto-1')
subdir('lto-2')
