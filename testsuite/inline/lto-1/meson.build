file = 'lto-1.c'

test_path = meson.current_source_dir() + '/' + file
outp_path = meson.current_build_dir() + '/' + file + '.log'

should_fail = returncode_to_bool[
    run_command('grep', '-nwI', '{ *dg-xfail *}', test_path, check: false).returncode()]
test(file, runtest, args : [ '-lto-test', '-inline-test', test_path, '-o', outp_path ],
           should_fail: should_fail)
