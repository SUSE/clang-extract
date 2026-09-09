//===- LaunchProcess.cpp - Implements Launching other processes *- C++ -*-===//
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

/* Author: Giuliano Belinassi  */

#include "LaunchProcess.hh"
#include "Error.hh"

#include <sys/wait.h>
#include <unistd.h>

static pid_t cc_pid;

static void forward_signal(int signal)
{
  /* Pass the signal to child process.  */
  if (cc_pid > 0) {
    kill(cc_pid, signal);
  }

  /* Then handle the signal myself.  */
  struct sigaction sa = {};
  sa.sa_handler = SIG_DFL;
  sigemptyset(&sa.sa_mask);
  sigaction(signal, &sa, nullptr);
  raise(signal);
}

static void install_sighandler(void (*func)(int))
{
  struct sigaction sa = {};
  sa.sa_handler = func;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT,  &sa, nullptr);
  sigaction(SIGTERM, &sa, nullptr);
}

/** Launch process in `path`, capturing stdout/stderr according to capture.  */
int launch_and_wait_process(const char *path, int argc, char *const argv[],
                            bool capture)
{
  int stdout_pipe[2], stderr_pipe[2];
  pid_t pid;

  assert(argv[argc-1] == NULL && "Missing ending NULL element in argv");

  if (capture) {
    if (pipe(stdout_pipe) == -1) {
      DiagsClass::Emit_Error("Unable to create stdout pipe for " + std::string(path));
      return -1;
    }

    if (pipe(stderr_pipe) == -1) {
      DiagsClass::Emit_Error("Unable to create stderr pipe for " + std::string(path));
      return -2;
    }
  }

  install_sighandler(forward_signal);

  cc_pid = pid = vfork();
  if (pid == -1) {
    DiagsClass::Emit_Error("Unable to fork into another process");
    return -3;
  }

  if (pid == 0) {
    /* Child.  */
    if (capture) {
      close(stdout_pipe[0]);
      close(stderr_pipe[0]);

      /* For redirecting stdout.  */
      if (dup2(stdout_pipe[1], STDOUT_FILENO) == -1) {
        exit(-126);
      }

        /* For redirecting stderr.  */
      if (dup2(stderr_pipe[1], STDERR_FILENO) == -1) {
        exit(-125);
      }

      close(stdout_pipe[1]);
      close(stderr_pipe[1]);
    }

    /* Launch process.  */
    execvp(path, argv);

    /* Should not happen.  */
    exit(-124);
  } else {
    /* Parent.  */

    if (capture) {
      char buffer[4096];
      ssize_t n;

      close(stdout_pipe[1]);
      close(stderr_pipe[1]);

      bool error_emited = false;
      while ((n = read(stdout_pipe[0], buffer, sizeof(buffer))) > 0) {
        if (!error_emited) {
          DiagsClass::Emit_Note("Program `" + std::string(path) + 
                                "` had the following messages in stdout:\n");
          error_emited = true;
        }
        fwrite(buffer, 1, n, stdout);
        fflush(stdout);
      }

      error_emited = false;
      while ((n = read(stderr_pipe[0], buffer, sizeof(buffer))) > 0) {
        if (!error_emited) {
          DiagsClass::Emit_Note("Program `" + std::string(path) + 
                                "` had the following messages in stderr:");
          error_emited = true;
        }

        fwrite(buffer, 1, n, stderr);
        fflush(stderr);
      }

      /* Close children communication.  */
      close(stdout_pipe[0]);
      close(stderr_pipe[0]);
    }

    /* Check children return status.  */
    int status;
    if (waitpid(pid, &status, 0) == -1) {
      DiagsClass::Emit_Error("Unable to wait for process with pid " + std::to_string(pid));
      cc_pid = -1;
      return -124;
    }

    install_sighandler(SIG_DFL);
    cc_pid = -1;

    if (WIFSIGNALED(status)) {
      DiagsClass::Emit_Error("Program " + std::string(path) + 
                            " killed by signal");
      return WTERMSIG(status);
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
      DiagsClass::Emit_Error("Program " + std::string(path) + 
                             " exited with a non-zero value!");
      return WEXITSTATUS(status);
    }
  }

  return 0;
}

#define EXEC_ARGV(args) (char *const *)(args)

int launch_and_wait_process(const char *path,
                            const std::vector<const char *> &argv, bool capture)
{
  /* exec expects that the ending element of argv is a NULL element.  */
  int argc = argv.size();
  if (argv[argc-1] != NULL) {
    std::vector<const char *> new_argv(argv);
    new_argv.push_back(NULL);
    argc++;
    return launch_and_wait_process(path, argc, EXEC_ARGV(new_argv.data()), capture);
  }

  return launch_and_wait_process(path, argc, EXEC_ARGV(argv.data()), capture);
}
