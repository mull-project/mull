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

void handle_alarm_signal(int signal, struct __siginfo *info, void *context) {
  exit(mull::ForkProcessSandbox::MullTimeoutCode);
}

mull::ExecutionResult
mull::ForkProcessSandbox::run(std::function<void (ExecutionResult *)> function,
                              long long timeoutMilliseconds) {

  char stderrFilename[] = "/tmp/mull.stderr.XXXXX";
  mktemp(stderrFilename);
  char stdoutFilename[] = "/tmp/mull.stdout.XXXXX";
  mktemp(stdoutFilename);

  /// Creating a memory to be shared between child and parent.
  void *sharedMemory = mmap(NULL,
                            sizeof(ExecutionResult),
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS,
                            -1,
                            0);

  ExecutionResult *sharedResult = new (sharedMemory) ExecutionResult();

  auto start = high_resolution_clock::now();
  const pid_t workerPID = mullFork("worker");
  if (workerPID == 0) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = &handle_alarm_signal;
    sigaction(SIGALRM, &action, NULL);

    useconds_t timeout = timeoutMilliseconds * 1000;
    ualarm(timeout, 0);

    freopen(stderrFilename, "w", stderr);
    freopen(stdoutFilename, "w", stdout);

    function(sharedResult);

    fflush(stderr);
    fflush(stdout);
    exit(MullExitCode);
  } else {
    int status = 0;
    pid_t pid = 0;
    while ( (pid = waitpid(workerPID, &status, 0)) == -1 ) {}

    auto elapsed = high_resolution_clock::now() - start;
    ExecutionResult abnormalResult;
    abnormalResult.RunningTime = duration_cast<std::chrono::milliseconds>(elapsed).count();
    abnormalResult.exitStatus = WEXITSTATUS(status);

    if (WIFSIGNALED(status)) {
      abnormalResult.Status = Crashed;
      *sharedResult = abnormalResult;
    }

    else if (WIFEXITED(status) && WEXITSTATUS(status) == MullTimeoutCode) {
      abnormalResult.Status = Timedout;
      *sharedResult = abnormalResult;
    }

    else if (WIFEXITED(status) && WEXITSTATUS(status) != MullExitCode) {
      abnormalResult.Status = AbnormalExit;
      *sharedResult = abnormalResult;
    }

    wait(0);

    ExecutionResult result = *sharedResult;

    result.stderrOutput = readFileAndUnlink(stderrFilename);
    result.stdoutOutput = readFileAndUnlink(stdoutFilename);

    int munmapResult = munmap(sharedMemory, sizeof(ExecutionResult));
    (void)munmapResult;

    /// Check that mummap succeeds:
    /// "On success, munmap() returns 0, on failure -1, and errno is set (probably to EINVAL)."
    /// http://linux.die.net/man/2/munmap
    assert(munmapResult == 0);

    return result;
  }
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
