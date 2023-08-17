#!/usr/bin/env python3

import sys

# Import the common test library.
sys.path.append('../lib/')
import libtest

# Parse given arguments
def parse_args():
    argc = len(sys.argv)
    input_path = None
    output_path = None

    skip = False

    for i in range(argc):
        if skip == True:
            skip = False
            continue

        if i + 1 < argc:
            if sys.argv[i] == "-o":
                output_path = sys.argv[i+1]
                skip = True # Skip next iteration
            else:
                input_path = sys.argv[i]
        else:
            input_path = sys.argv[i]

    return (input_path, output_path)

if __name__ == '__main__':
    input_path, logfile_path = parse_args()
    if input_path == None:
        print("No input file was given")
        exit(1)

    if logfile_path == None:
        print("No output logfile was given")
        exit(1)

    # Run test.
    test = libtest.UnitTest(input_path, logfile_path)
    test.run_test()

    exit(0)
