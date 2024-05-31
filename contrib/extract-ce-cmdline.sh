#!/bin/sh
#
# This project is licensed under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# \file
# Script to extract clang-extract command-line from klp-build ce log files.
#

# Name of the script.
PROGNAME=`basename "$0"`

# The input file
INPUT_FILE=''

# The output file
OUTPUT_FILE=''

# With GDB call?
WITH_GDB=0

print_help_message()
{
  echo "Setup script to extract clang-extract command line from klp-build CE log"
  echo ""
  echo "Usage: $PROGNAME ce.out.txt <switches>"
  echo "where <switches>"
  echo "  -o=OUTPUT_FILE            Output to write. Empty or '-' will print to stdout."
  echo "  --with-gdb                clang-extract will be called within gdb."
  echo ""
}

# Parse the arguments given to script.
parse_program_argv()
{
  # Parse arguments provided by user.
  for i in "$@"; do
    case $i in
      -o=*)
        OUTPUT_FILE="${i#*=}"
        shift
        ;;
      --with-gdb)
        WITH_GDB=1
        shift
        ;;
      -*|--*)
        echo "Unknown option $i"
        echo ""
        print_help_message
        shift
        exit 1
        ;;
      *)
        INPUT_FILE=${i}
        shift
        ;;
    esac
  done

  # Bails out if no input file was given
  if [ "x$INPUT_FILE" == "x" ]; then
    echo "Error: no input file.\n"
    print_help_message
    exit 1
  fi

  # Set output to /dev/stdout if no output file was given, or if '-o -'
  # was passed.
  if [ "x$OUTPUT_FILE" == "x" -o "$OUTPUT_FILE" == "-" ]; then
    OUTPUT_FILE="/dev/stdout"
  fi
}

generate_script()
{
  # Get the pwd where clang-extract was run.
  local string_pwd=$(sed -nr 's/Executing ce on (.*)/\1/p' $INPUT_FILE)
  local found_ce=0

  echo '#!/bin/bash'       >  $OUTPUT_FILE
  echo "set -e"            >> $OUTPUT_FILE
  echo "pushd $string_pwd" >> $OUTPUT_FILE
  while IFS= read -r line; do
    # If we did not reach the error state, then we are parsing clang-extract
    # options.

    if [ $found_ce -eq 0 ]; then
      # Find the begining of the clang-extract commands.
      echo "$line" | grep 'clang-extract$' > /dev/null
      if [ $? -eq 0 ]; then
        found_ce=1
        if [ $WITH_GDB -eq 1 ]; then
          echo 'gdb -args ' $line '\' >> $OUTPUT_FILE
        else
          echo $line '\' >> $OUTPUT_FILE
        fi
      fi
    else
      # Find the end of the clang-extract command lines.
      echo "$line" | grep -E '.*warning:.*|.*error:.*|.*note:.*|^In file included*' > /dev/null
      if [ $? -eq 0 ]; then
        break
      fi

      echo $line '\' >> $OUTPUT_FILE
    fi
  done < $INPUT_FILE

  # Newline to ignore the fact that the last command ends with a '\'
  echo '' >> $OUTPUT_FILE

  # Pop the directory to go back where this script was called.
  echo 'popd' >> $OUTPUT_FILE

  # Newline to comply with POSIX text files.
  echo '' >> $OUTPUT_FILE
  # Set file as executable
  if [ "$OUTPUT_FILE" != "/dev/null" ]; then
    chmod +x $OUTPUT_FILE
  fi
}

# Parse the input file

# Script entry point.
main()
{
  parse_program_argv $*
  generate_script
}

main $*
