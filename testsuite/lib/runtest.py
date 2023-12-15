#!/usr/bin/env python3

import sys

# Import the common test library.
sys.path.append('../lib/')
import libtest

is_inline_test = False
is_lto_test = False

# Parse given arguments
def parse_args():
    global is_inline_test
    global is_lto_test

    argc = len(sys.argv)
    input_path = None
    output_path = None
    binaries_path = "../../build/"


    skip = False

    for i in range(argc):
        if skip == True:
            skip = False
            continue

        if sys.argv[i] == "-inline-test":
            is_inline_test = True
        elif sys.argv[i] == "-lto-test":
            is_lto_test = True
        elif i + 1 < argc:
            if sys.argv[i] == "-o":
                output_path = sys.argv[i+1]
                skip = True # Skip next iteration
            elif sys.argv[i] == "-bin-path":
                binaries_path = sys.argv[i+1]
            else:
                input_path = sys.argv[i]
        else:
            input_path = sys.argv[i]

    return (input_path, output_path, binaries_path)

if __name__ == '__main__':
    input_path, logfile_path, binaries_path = parse_args()
    if input_path == None:
        print("No input file was given")
        exit(1)

    if logfile_path == None:
        print("No output logfile was given")
        exit(1)

    # Run test.
    test = libtest.UnitTest(input_path, logfile_path, binaries_path)
    if is_inline_test:
        test.run_inline_test(is_lto_test)
    else:
        test.run_test(is_lto_test)

    exit(0)
