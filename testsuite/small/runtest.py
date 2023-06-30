#!/usr/bin/env python3

import os
import pathlib
import re
import signal
import subprocess
import sys
import time
import tempfile

# Third-party libraries
import pexpect
import psutil

# Define where the build directory is
builddir = ''

# Define path to clang-extract
clang_extract = builddir + '../../clang-extract'

# Extract extra options from the file
def extract_options(filename):
  p = re.compile('{ *dg-options "(.*)" *}')
  with open(filename, mode="rt", encoding="utf-8") as file:
    line = file.read()
    matches = re.search(p, line)
    if matches is not None:
      matches = matches.group(1)

  if matches is None:
    return ''
  return matches

def extract_must_have(filename):
  p = re.compile('{ *dg-final *{ *scan-tree-dump *"(.*)" *} *}')
  matches = []
  with open(filename, mode="rt", encoding="utf-8") as file:
    line = file.read()
    matched = re.search(p, line)
    if matched is not None:
      matches.append(matched.group(1))

  return matches

def extract_must_not_have(filename):
  p = re.compile('{ *dg-final *{ *scan-tree-dump-not *"(.*)" *} *}')
  matches = []
  with open(filename, mode="rt", encoding="utf-8") as file:
    line = file.read()
    if matched is not None:
      matches.append(matched.group(1))

  return matches

def check_output(filename, must_have, must_not_have):
  # Compile regexes
  must_have_regexes = []
  must_not_have_regexes = []

  for x in range(must_have):
    p = re.compile(x)
    must_have_regexes.append(p)

  for x in range(must_not_have):
    p = re.compile(x)
    must_not_have_regexes.append(p)

  with open(filename, mode="rt", encoding="utf-8") as file:
    line = file.read()
    for p in range(must_have_regexes):
      matched = re.match(p, line)
      if matched is None:
        return False

    for p in range(must_not_have_regexes):
      matched = re.match(p, line)
      if matched is not None:
        return False

  return True

def run_clang_extract(testfile, out):
  command = [ clang_extract, '-DCE_OUTPUT_FILE=' + out, testfile ]

  command.append(extract_options(testfile))

  tool = subprocess.run(command, timeout=10)


def parse_args():
  argc = len(sys.argv)
  input_path = None
  output_path = None

  for i in range(argc):
      if i + 1 < argc:
        if sys.argv[i] == "-o":
          output_path = sys.argv[i+1]
          i = i + 1
        else:
          input_path = sys.argv[i]
      else:
        input_path = sys.argv[i]

  return (input_path, output_path)


def print_result(result, input_path):
  if result == 0:
    print("SUCCESS: ", input_path)
  if result == 77:
    print("SKIPPED: ", input_path)
  else:
    print("FAIL: ", input_path)

remove_output = False
input_path, output_path = parse_args()
if input_path == None:
  print("No input file was given")
  exit(1)
if output_path == None:
  output_path = '/tmp/' + next(tempfile._get_candidate_names()) + '.CE.c'
  remove_output = True

result = run_clang_extract(input_path, output_path)
print_result(result, input_path)

if remove_output is True:
  try:
    os.remove(output_path)
  except FileNotFoundError:
    pass

exit(result)
