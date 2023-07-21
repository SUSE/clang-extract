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

RESET  = '\033[0m'
GREEN  = '\033[32m'
HGREEN = '\033[0;92m'
RED    = '\033[91m'
HRED   = '\033[0;91m'
YELLOW = '\033[93m'
BLUE   = '\033[34m'

# Logs messages into file.
class Log:
    def __init__(self, output_path):
        self.output_path = output_path
        self.file = open(output_path, "w")

    def __del__(self):
        self.file.close()

    def print(self, s):
        self.file.write(s + '\n')

class UnitTest:
    def __init__(self, test_path, log_path):
        self.log = Log(log_path)

        self.test_path = test_path
        self.file = open(test_path, "r")
        self.file_content = self.file.read()
        self.lines = self.file_content.split('\n')

        self.options = self.extract_options()
        self.must_have = self.extract_must_have()
        self.must_not_have = self.extract_must_not_have()
        self.error_msgs = self.extract_error_msgs()
        self.warning_msgs = self.extract_warning_msgs()

        self.must_have_regexes, self.must_not_have_regexes, self.error_msgs_regexes, self.warning_msgs_regexes = self.compile_regexes()

    # Extract options passed through dg-options.
    def extract_options(self):
        p = re.compile('{ *dg-options "(.*)" *}')
        matches = re.search(p, self.file_content)
        if matches is not None:
            matches = matches.group(1)
            return matches.split()

        self.log.print("WARNING: No dg-options given")
        return []

    # Extract rules that must be in the clang-extract output.
    def extract_must_have(self):
        p = re.compile('{ *dg-final *{ *scan-tree-dump *"(.*)" *} *}')

        matches = []
        for line in self.lines:
            matched = re.search(p, line)
            if matched is not None:
                matches.append(matched.group(1))
        return matches

    # Extract error message.
    def extract_error_msgs(self):
        p = re.compile('{ *dg-error *"(.*)" *}')

        matches = []
        for line in self.lines:
            matched = re.search(p, line)
            if matched is not None:
                regex = "error: .*" + matched.group(1)
                matches.append(regex)
        return matches

    # Extract warning message.
    def extract_warning_msgs(self):
        p = re.compile('{ *dg-warning *"(.*)" *}')

        matches = []
        for line in self.lines:
            matched = re.search(p, line)
            if matched is not None:
                regex = "warning: .*" + matched.group(1)
                matches.append(regex)
        return matches

    # Extract rules that must NOT be in the clang-extract output.
    def extract_must_not_have(self):
        p = re.compile('{ *dg-final *{ *scan-tree-dump-not *"(.*)" *} *}')

        matches = []
        for line in self.lines:
            matched = re.search(p, line)
            if matched is not None:
                matches.append(matched.group(1))

        return matches

    # Flag that test must XFAIL
    def extract_should_xfail(self):
        p = re.compile('{ *dg-xfail *}')
        matched = re.search(p, self.file_content)
        if matched is not None:
            return True

        return False

    # Compile the regexes in scan-tree-dump and scan-tree-dump-not.
    def compile_regexes(self):
        must_have_regexes = []
        must_not_have_regexes = []
        error_msgs_regexes = []
        warning_msgs_regexes = []

        for x in self.must_have:
            p = re.compile(x)
            must_have_regexes.append(x)

        for x in self.must_not_have:
            p = re.compile(x)
            must_not_have_regexes.append(x)

        for x in self.error_msgs:
            p = re.compile(x)
            error_msgs_regexes.append(x)

        for x in self.warning_msgs:
            p = re.compile(x)
            warning_msgs_regexes.append(x)

        return must_have_regexes, must_not_have_regexes, error_msgs_regexes, warning_msgs_regexes

    def print_result(self, result, should_xfail = False):
        if result == 0:
            if should_xfail == True:
                print(HRED, "XSUCCESS:", RESET, self.test_path)
                self.log.print("XSUCCESS:" + self.test_path)
            else:
                print(GREEN, "SUCCESS: ", RESET, self.test_path)
                self.log.print("SUCCESS: " + self.test_path)
        elif result == 77:
            print(YELLOW, "SKIPPED:  ", RESET, self.test_path)
            self.log.print("SKIPPED:  " + self.test_path)
        else:
            if should_xfail == True:
                print(HGREEN, "XFAIL:   ", RESET, self.test_path)
                self.log.print("XFAIL: " + self.test_path)
            else:
                print(RED, "FAIL:    ", RESET, self.test_path)
                self.log.print("FAIL: " + self.test_path)

    def check_founds(self, founds, not_found_msg, rules):
        for i in range(len(founds)):
            x = founds[i]
            if x is False:
                self.log.print(not_found_msg + rules[i])
                return False
        return True

    def check_terminal_output(self, terminal_output):
        founds = [False] * len(self.error_msgs_regexes)
        for i in range(len(self.error_msgs_regexes)):
            p = self.error_msgs_regexes[i]
            matched = re.search(p, terminal_output)
            if matched is not None:
                founds[i] = True

        if self.check_founds(founds, "Error message not found: ",
                             self.error_msgs) == False:
            return False

        founds = [False] * len(self.warning_msgs_regexes)

        for i in range(len(self.warning_msgs_regexes)):
            p = self.error_msgs_regexes[i]
            matched = re.search(p, terminal_output)
            if matched is not None:
                founds[i] = True

        if self.check_founds(founds, "Warning message not found: ",
                             self.warning_msgs) == False:
            return False

        return True


    # Check if clang-extract output matches the rules in the test.
    def check_output(self, output_file):

        try:
            founds = [False] * len(self.must_have_regexes)
            with open(output_file, mode="rt", encoding="utf-8") as file:
                line = file.read()

                self.log.print("clang-extract output:")
                self.log.print(line)

                self.log.print("-----")

                for i in range(len(self.must_have_regexes)):
                    p = self.must_have_regexes[i]
                    matched = re.search(p, line)
                    if matched is not None:
                        founds[i] = True

                for i in range(len(self.must_not_have_regexes)):
                    p = self.must_not_have_regexes[i]
                    matched = re.search(p, line)
                    if matched is not None:
                        self.log.print("Must not have pattern found: " +
                                       self.must_not_have[i])
                        return False

                if self.check_founds(founds, "Must have pattern not found: ",
                                self.must_have) == False:
                    return False
        except FileNotFoundError:
            return False

        return True

    # Indeed run the test.
    def run_test(self):
        clang_extract = '../../clang-extract'
        ce_output_path = '/tmp/' + next(tempfile._get_candidate_names()) + '.CE.c'

        command = [ clang_extract, '-DCE_OUTPUT_FILE=' + ce_output_path,
                    self.test_path ]
        command.extend(self.options)

        tool = subprocess.run(command, timeout=10, stderr=subprocess.STDOUT,
                              stdout=subprocess.PIPE)

        self.log.print("terminal output of clang-extract:")
        self.log.print(tool.stdout.decode())

        should_xfail = self.extract_should_xfail()

        if self.check_terminal_output(tool.stdout.decode()) == False:
            self.print_result(1)
            return 1

        # Only check the output if there is no error message expected.
        if len(self.error_msgs) == 0:
            if self.check_output(ce_output_path) == False:
                self.print_result(1, should_xfail)
                return 1

            if tool.returncode != 0:
                self.print_result(tool.returncode, should_xfail)
                return tool.returncode

        try:
            os.remove(ce_output_path)
        except FileNotFoundError:
            pass

        self.print_result(0, should_xfail)

        return 0
