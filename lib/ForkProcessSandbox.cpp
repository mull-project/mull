#include "ForkProcessSandbox.h"

#include "Logger.h"
#include "TestResult.h"

#include <errno.h>
#include <chrono>
#include <signal.h>
#include <string.h>
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

static pid_t mullFork(const char *processName) {
  static int childrenCount = 0;
  childrenCount++;
  const pid_t pid = fork();
  if (pid == -1) {
    mull::Logger::error() << "Failed to create " << processName
                            << " after creating " << childrenCount
                            << " child processes\n";
    mull::Logger::error() << strerror(errno) << "\n";
    mull::Logger::error() << "Shutting down\n";
    exit(1);
  }
  return pid;
}

static std::string readFileAndUnlink(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("fopen");
    exit(1);
  }
  if (unlink(filename) == -1) {
    perror("unlink");
  }

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char *buffer = (char *)malloc(sizeof(char) * (size + 1));
  fread(buffer, sizeof(char), size, file);
  buffer[size] = '\0';
  std::string output(buffer);
  free(buffer);

  return output;
}

mull::ExecutionResult
mull::ForkProcessSandbox::run(std::function<void (ExecutionResult *)> function,
                              long long timeoutMilliseconds) {

  char stderrFilename[] = "/tmp/mull.stderr.XXXXX";
  mktemp(stderrFilename);
  char stdoutFilename[] = "/tmp/mull.stdout.XXXXX";
  mktemp(stdoutFilename);

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
      freopen(stderrFilename, "w", stderr);
      freopen(stdoutFilename, "w", stdout);

      function(sharedResult);

      fflush(stderr);
      fflush(stdout);
      exit(MullExitCode);
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
      }

      else if (WIFEXITED(status) && WEXITSTATUS(status) != MullExitCode) {
        auto elapsed = high_resolution_clock::now() - start;
        ExecutionResult result;
        result.RunningTime = duration_cast<std::chrono::milliseconds>(elapsed).count();
        result.Status = AbnormalExit;
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

  wait(0);

  ExecutionResult result = *sharedResult;

  result.stderrOutput = readFileAndUnlink(stderrFilename);
  result.stdoutOutput = readFileAndUnlink(stdoutFilename);

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
