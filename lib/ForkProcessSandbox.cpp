
#include "ForkProcessSandbox.h"

#include "TestResult.h"

#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

pid_t mutangFork(const char *processName) {
  static int childrenCount = 0;
  childrenCount++;
  const pid_t pid = fork();
  if (pid == -1) {
    printf("Failed to create %s after creating %d child processes\n",
           processName, childrenCount);
    printf("%s\n", strerror(errno));
    printf("Shutting down\n");
    exit(1);
  }
  return pid;
}

Mutang::ExecutionResult Mutang::ForkProcessSandbox::run(std::function<void (ExecutionResult *)> function) {

  void *SharedMemory = mmap(NULL,
                            sizeof(ExecutionResult),
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS,
                            -1,
                            0);

  ExecutionResult *sharedResult = new (SharedMemory) ExecutionResult();

  const pid_t watchdogPID = mutangFork("watchdog");
  if (watchdogPID == 0) {

    const pid_t timerPID = mutangFork("timer");
    if (timerPID == 0) {
      sleep(3);
      exit(0);
    }

    const pid_t workerPID = mutangFork("worker");
    if (workerPID == 0) {
      function(sharedResult);
      exit(0);
    }

    int status = 0;
    const pid_t exitedPID = waitpid(WAIT_ANY, &status, 0);
    if (exitedPID == timerPID) {
      /// Timer Process finished first, meaning that the worker timed out
      kill(workerPID, SIGKILL);
      ExecutionResult result;
      result.Status = Timedout;
      *sharedResult = result;
    } else {
      kill(timerPID, SIGKILL);
      /// Worker Process finished first
      /// Need to check whether it has signaled (crashed) or finished normally
      if (WIFSIGNALED(status)) {
        ExecutionResult result;
        result.Status = Crashed;
        *sharedResult = result;
      }
    }

    wait(nullptr);
    exit(0);
  } else {
    pid_t pid = 0;
    while ( (pid = waitpid(watchdogPID, 0, 0)) == -1 ) {}
  }

  ExecutionResult result = *sharedResult;

  int munmapResult = munmap(SharedMemory, sizeof(ExecutionResult));
  (void)munmapResult;

  /// Check that mummap succeeds:
  /// "On success, munmap() returns 0, on failure -1, and errno is set (probably to EINVAL)."
  /// http://linux.die.net/man/2/munmap
  assert(munmapResult == 0);

  return result;
}

Mutang::ExecutionResult Mutang::NullProcessSandbox::run(std::function<void (ExecutionResult *)> function) {
  void *SharedMemory = malloc(sizeof(ExecutionResult));

  ExecutionResult *SharedResult = new (SharedMemory) ExecutionResult();

  function(SharedResult);

  ExecutionResult Result = *SharedResult;

  free(SharedMemory);

  return Result;
}
