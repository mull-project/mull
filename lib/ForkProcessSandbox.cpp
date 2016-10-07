
#include "ForkProcessSandbox.h"

#include "TestResult.h"

#include <sys/mman.h>
#include <unistd.h>

Mutang::ExecutionResult Mutang::ForkProcessSandbox::run(std::function<void (ExecutionResult *)> Func) {

  void *SharedMemory = mmap(NULL,
                            sizeof(ExecutionResult),
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS,
                            -1,
                            0);

  ExecutionResult *SharedResult = new (SharedMemory) ExecutionResult();

  static unsigned long long kaunt = 0;

  int ChildPid = fork();
  kaunt++;
  if (ChildPid == -1) {
    printf("child pid -1\n");
    printf("after %llu forks\n", kaunt);
    printf("reason: %s\n", strerror(errno));
    exit(1);
  }

  else if (ChildPid == 0) {
    Func(SharedResult);
    exit(0);
  }

  else {
    /// TODO: Handle at least 3 cases:
    /// 1) Child process crashes
    /// 2) Child process exits 1
    /// 3) Child process hangs
    int stat_loc = 0;
    waitpid(ChildPid, &stat_loc, 0);
  }

  ExecutionResult Result = *SharedResult;

  int MunmapResult = munmap(SharedMemory, sizeof(ExecutionResult));

  /// Check that mummap succeeds:
  /// "On success, munmap() returns 0, on failure -1, and errno is set (probably to EINVAL)."
  /// http://linux.die.net/man/2/munmap
  assert(MunmapResult == 0);

  return Result;
}

Mutang::ExecutionResult Mutang::NullProcessSandbox::run(std::function<void (ExecutionResult *)> Func) {
  void *SharedMemory = malloc(sizeof(ExecutionResult));

  ExecutionResult *SharedResult = new (SharedMemory) ExecutionResult();

  Func(SharedResult);

  ExecutionResult Result = *SharedResult;

  free(SharedMemory);

  return Result;
}
