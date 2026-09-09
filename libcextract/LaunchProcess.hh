//===- LaunchProcess.hh - Implements Launching other processes *- C++ -*-===//
//
// This project is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This file implements functions that calls Linux syscalls to launch other
/// processes and do basic Inter-process communication.
//
//===----------------------------------------------------------------------===//

#pragma once

#include <vector>

/** Launch external program.  */
int launch_and_wait_process(const char *path, int argc, char *const argv[],
                            bool capture = false);

int launch_and_wait_process(const char *path,
                            const std::vector<const char *> &argv,
                            bool capture = false);
