#include "ForkProcessSandbox.h"

#include "Logger.h"
#include "ExecutionResult.h"

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
    printf("fopen: %s\n", filename);
    return std::string("Mull error: could not read output");
  }
  if (unlink(filename) == -1) {
    perror("unlink");
    printf("unlink: %s\n", filename);
  }

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char *buffer = (char *)malloc(sizeof(char) * (size + 1));
  fread(buffer, sizeof(char), size, file);
  buffer[size] = '\0';
  std::string output(buffer);
  free(buffer);
  fclose(file);

  return output;
}

void handle_alarm_signal(int signal, siginfo_t *info, void *context) {
  _exit(mull::ForkProcessSandbox::MullTimeoutCode);
}

mull::ExecutionResult
mull::ForkProcessSandbox::run(std::function<ExecutionStatus (void)> function,
                              long long timeoutMilliseconds) {

  char stderrFilename[] = "/tmp/mull.stderr.XXXXXX";
  mktemp(stderrFilename);

  char stdoutFilename[] = "/tmp/mull.stdout.XXXXXX";
  mktemp(stdoutFilename);

  /// Creating a memory to be shared between child and parent.
  ExecutionStatus *sharedStatus = (ExecutionStatus *)mmap(NULL,
                                                          sizeof(ExecutionStatus),
                                                          PROT_READ | PROT_WRITE,
                                                          MAP_SHARED | MAP_ANONYMOUS,
                                                          -1,
                                                          0);

  auto start = high_resolution_clock::now();
  const pid_t workerPID = mullFork("worker");
  if (workerPID == 0) {
    freopen(stderrFilename, "w", stderr);
    freopen(stdoutFilename, "w", stdout);

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = &handle_alarm_signal;
    sigaction(SIGALRM, &action, NULL);

    useconds_t timeout = timeoutMilliseconds * 1000;
    ualarm(timeout, 0);

    *sharedStatus = function();

    fflush(stderr);
    fflush(stdout);
    _exit(MullExitCode);
  } else {
    int status = 0;
    pid_t pid = 0;
    while ( (pid = waitpid(workerPID, &status, 0)) == -1 ) {}

    auto elapsed = high_resolution_clock::now() - start;
    ExecutionResult result;
    result.runningTime = duration_cast<std::chrono::milliseconds>(elapsed).count();
    result.exitStatus = WEXITSTATUS(status);
    result.stderrOutput = readFileAndUnlink(stderrFilename);
    result.stdoutOutput = readFileAndUnlink(stdoutFilename);
    result.status = *sharedStatus;

    int munmapResult = munmap(sharedStatus, sizeof(ExecutionStatus));

    /// Check that mummap succeeds:
    /// "On success, munmap() returns 0, on failure -1, and errno is set (probably to EINVAL)."
    /// http://linux.die.net/man/2/munmap
    assert(munmapResult == 0);
    (void)munmapResult;

    if (WIFSIGNALED(status)) {
      result.status = Crashed;
    }

    else if (WIFEXITED(status) && WEXITSTATUS(status) == MullTimeoutCode) {
      result.status = Timedout;
    }

    else if (WIFEXITED(status) && WEXITSTATUS(status) != MullExitCode) {
      result.status = AbnormalExit;
    }

    return result;
  }
}

mull::ExecutionResult mull::NullProcessSandbox::run(std::function<ExecutionStatus (void)> function,
                                                    long long timeoutMilliseconds) {
  ExecutionResult result;
  result.status = function();
  return result;
}
