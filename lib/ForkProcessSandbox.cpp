#include "ForkProcessSandbox.h"

#include "Logger.h"
#include "TestResult.h"

#include <chrono>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

using namespace std::chrono;

enum {
  FPSPipeWrite = 1,
  FPSPipeRead = 0
};

pid_t mullFork(const char *processName) {
  static int childrenCount = 0;
  childrenCount++;
  const pid_t pid = fork();
  if (pid == -1) {
    mull::Logger::error() << "Failed to create " << processName
                            << " after creating " << childrenCount
                            << " child processes\n";
    mull::Logger::error() << "Shutting down\n";
    exit(1);
  }
  return pid;
}

mull::ExecutionResult
mull::ForkProcessSandbox::run(std::function<void (ExecutionResult *)> function,
                                long long timeoutMilliseconds) {

  /// Preparing pipes for child process to write to and parent process to read from.
  int stdout_file_descriptor[2];
  int stderr_file_descriptor[2];
  if (pipe(stdout_file_descriptor) == -1) {
    perror("stdout pipe");
    exit(1);
  }

  if (pipe(stderr_file_descriptor) == -1) {
    perror("stderr_pipe");
    exit(1);
  }

  /// Creating a memory to be shared between child and parent.
  void *SharedMemory = mmap(NULL,
                            sizeof(ExecutionResult),
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS,
                            -1,
                            0);

  ExecutionResult *sharedResult = new (SharedMemory) ExecutionResult();

  const pid_t watchdogPID = mullFork("watchdog");
  if (watchdogPID == 0) {

    const pid_t timerPID = mullFork("timer");
    if (timerPID == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMilliseconds));
      exit(0);
    }

    const pid_t workerPID = mullFork("worker");

    auto start = high_resolution_clock::now();

    if (workerPID == 0) {
      while ((dup2(stdout_file_descriptor[FPSPipeWrite], STDOUT_FILENO) == -1) && (errno == EINTR)) { }
      while ((dup2(stderr_file_descriptor[FPSPipeWrite], STDERR_FILENO) == -1) && (errno == EINTR)) { }

      close(stdout_file_descriptor[FPSPipeWrite]);
      close(stdout_file_descriptor[FPSPipeRead]);
      close(stderr_file_descriptor[FPSPipeWrite]);
      close(stderr_file_descriptor[FPSPipeRead]);

      //      execl("/bin/pwd", "pwd", (char*)0);

      function(sharedResult);

      exit(0);
    }

    int status = 0;
    const pid_t exitedPID = waitpid(WAIT_ANY, &status, 0);
    if (exitedPID == timerPID) {
      /// Timer Process finished first, meaning that the worker timed out
      kill(workerPID, SIGKILL);
      auto elapsed = high_resolution_clock::now() - start;

      ExecutionResult result;
      result.Status = Timedout;
      result.RunningTime = duration_cast<std::chrono::milliseconds>(elapsed).count();
      *sharedResult = result;
    } else if (exitedPID == workerPID) {
      kill(timerPID, SIGKILL);

      /// Worker Process finished first
      /// Need to check whether it has signaled (crashed) or finished normally
      if (WIFSIGNALED(status)) {
        auto elapsed = high_resolution_clock::now() - start;
        ExecutionResult result;
        result.RunningTime = duration_cast<std::chrono::milliseconds>(elapsed).count();
        result.Status = Crashed;
        *sharedResult = result;
      } else if (WIFEXITED(status) &&
                 (WEXITSTATUS(status) != 0 || sharedResult->Status == Invalid)) {
        auto elapsed = high_resolution_clock::now() - start;
        ExecutionResult result;
        result.RunningTime = duration_cast<std::chrono::milliseconds>(elapsed).count();
        result.Status = Failed;
        *sharedResult = result;
      }
    } else {
      llvm_unreachable("Should not reach!");
    }

    wait(nullptr);
    exit(0);
  } else {
    pid_t pid = 0;
    while ( (pid = waitpid(watchdogPID, 0, 0)) == -1 ) {}
  }

  close(stdout_file_descriptor[FPSPipeWrite]);
  close(stderr_file_descriptor[FPSPipeWrite]);

  char child_stdout_buffer[4096];
  char child_stderr_buffer[4096];

  std::string stdoutOutput;
  std::string stderrOutput;

  while (1) {
    ssize_t count = read(stdout_file_descriptor[0], child_stdout_buffer, sizeof(child_stdout_buffer));
    if (count == -1) {
      if (errno == EINTR) {
        continue;
      } else {
        perror("read");
        exit(1);
      }
    } else if (count == 0) {
      break;
    } else {
      stdoutOutput.append(child_stdout_buffer, 0, count);
    }
  }

  while (1) {
    ssize_t count = read(stderr_file_descriptor[0], child_stderr_buffer, sizeof(child_stderr_buffer));
    if (count == -1) {
      if (errno == EINTR) {
        continue;
      } else {
        perror("read");
        exit(1);
      }
    } else if (count == 0) {
      break;
    } else {
      stderrOutput.append(child_stderr_buffer, 0, count);
    }
  }

  close(stdout_file_descriptor[FPSPipeRead]);
  close(stderr_file_descriptor[FPSPipeRead]);

  wait(0);

  ExecutionResult result = *sharedResult;

  result.stdoutOutput = stdoutOutput;
  result.stderrOutput = stderrOutput;

  int munmapResult = munmap(SharedMemory, sizeof(ExecutionResult));
  (void)munmapResult;

  /// Check that mummap succeeds:
  /// "On success, munmap() returns 0, on failure -1, and errno is set (probably to EINVAL)."
  /// http://linux.die.net/man/2/munmap
  assert(munmapResult == 0);

  return result;
}

mull::ExecutionResult mull::NullProcessSandbox::run(std::function<void (ExecutionResult *)> function,
                                                        long long timeoutMilliseconds) {
  void *SharedMemory = malloc(sizeof(ExecutionResult));

  ExecutionResult *SharedResult = new (SharedMemory) ExecutionResult();

  function(SharedResult);

  ExecutionResult Result = *SharedResult;

  free(SharedMemory);

  return Result;
}
