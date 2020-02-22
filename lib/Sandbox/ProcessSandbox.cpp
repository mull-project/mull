#include "mull/Sandbox/ProcessSandbox.h"

#include "mull/Diagnostics/Diagnostics.h"
#include "mull/ExecutionResult.h"

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <llvm/Support/FileSystem.h>
#include <sstream>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

using namespace std::chrono;

static pid_t mullFork(mull::Diagnostics &diagnostics, const char *processName) {
  const pid_t pid = fork();
  if (pid == -1) {
    std::stringstream stringstream;
    stringstream << "Failed to create " << processName << "\n"
                 << strerror(errno) << "\n"
                 << "Shutting down";
    diagnostics.error(stringstream.str());
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
  _exit(mull::ForkTimerSandbox::MullTimeoutCode);
}

void handle_timeout(long long timeoutMilliseconds) {
  struct sigaction action {};
  memset(&action, 0, sizeof(action));
  action.sa_sigaction = &handle_alarm_signal;
  if (sigaction(SIGALRM, &action, nullptr) != 0) {
    perror("sigaction");
    abort();
  }

  struct itimerval timer {};
  timer.it_value.tv_sec = timeoutMilliseconds / 1000;
  /// Cut off seconds, and convert what's left into microseconds
  timer.it_value.tv_usec = (timeoutMilliseconds % 1000) * 1000;

  /// Do not repeat
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;

  if (setitimer(ITIMER_REAL, &timer, nullptr) != 0) {
    perror("setitimer");
    abort();
  }
}

mull::ExecutionResult mull::ForkTimerSandbox::run(Diagnostics &diagnostics,
                                                  std::function<ExecutionStatus(void)> function,
                                                  long long timeoutMilliseconds) const {
  llvm::SmallString<32> stderrFilename;
  llvm::sys::fs::createUniqueFile("/tmp/mull.stderr.%%%%%%", stderrFilename);

  llvm::SmallString<32> stdoutFilename;
  llvm::sys::fs::createUniqueFile("/tmp/mull.stdout.%%%%%%", stdoutFilename);

  /// Creating a memory to be shared between child and parent.
  auto *sharedStatus = (ExecutionStatus *)mmap(
      nullptr, sizeof(ExecutionStatus), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  auto start = high_resolution_clock::now();
  const pid_t workerPID = mullFork(diagnostics, "worker");
  if (workerPID == 0) {
    /// TODO: There is an issue with freopen: it hangs once in a while, so as a hack we add the
    /// timeout handling before calling freopen. There is a bug somewhere and it needs to be fixed.
    /// It happens only on macOS, here is the most relevant thread:
    /// hangs in flockfile() during fread() or fclose()
    /// https://groups.google.com/forum/#!topic/darwin-dev/e5qPebpfvYM
    handle_timeout(timeoutMilliseconds);

    freopen(stderrFilename.c_str(), "w", stderr);
    freopen(stdoutFilename.c_str(), "w", stdout);

    *sharedStatus = function();

    fflush(stderr);
    fflush(stdout);
    _exit(MullExitCode);
  } else {
    int status = 0;
    pid_t pid = 0;
    while ((pid = waitpid(workerPID, &status, 0)) == -1) {
    }

    auto elapsed = high_resolution_clock::now() - start;
    ExecutionResult result;
    result.runningTime = duration_cast<std::chrono::milliseconds>(elapsed).count();
    result.exitStatus = WEXITSTATUS(status);
    result.stderrOutput = readFileAndUnlink(stderrFilename.c_str());
    result.stdoutOutput = readFileAndUnlink(stdoutFilename.c_str());
    result.status = *sharedStatus;

    if (WIFSIGNALED(status)) {
      result.status = Crashed;
    }

    else if (WIFEXITED(status) && WEXITSTATUS(status) == MullTimeoutCode) {
      result.status = Timedout;
    }

    else if (WIFEXITED(status) && WEXITSTATUS(status) != MullExitCode) {
      result.status = AbnormalExit;
    }

    else if (WIFEXITED(status) && WEXITSTATUS(status) == MullExitCode && *sharedStatus == Invalid) {
      result.status = Passed;
    }

    /// Check that mummap succeeds:
    /// "On success, munmap() returns 0, on failure -1, and errno is set
    /// (probably to EINVAL)." http://linux.die.net/man/2/munmap
    int munmapResult = munmap(sharedStatus, sizeof(ExecutionStatus));
    assert(munmapResult == 0);
    (void)munmapResult;

    return result;
  }
}

mull::ExecutionResult mull::ForkWatchdogSandbox::run(Diagnostics &diagnostics,
                                                     std::function<ExecutionStatus(void)> function,
                                                     long long timeoutMilliseconds) const {
  llvm::SmallString<32> stderrFilename;
  llvm::sys::fs::createUniqueFile("/tmp/mull.stderr.%%%%%%", stderrFilename);

  llvm::SmallString<32> stdoutFilename;
  llvm::sys::fs::createUniqueFile("/tmp/mull.stdout.%%%%%%", stdoutFilename);

  /// Creating a memory to be shared between child and parent.
  void *sharedMemory = mmap(
      nullptr, sizeof(ExecutionResult), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  ExecutionResult *sharedResult = new (sharedMemory) ExecutionResult();

  const pid_t watchdogPID = mullFork(diagnostics, "watchdog");
  if (watchdogPID == 0) {

    const pid_t timerPID = mullFork(diagnostics, "timer");
    if (timerPID == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMilliseconds));
      exit(0);
    }

    const pid_t workerPID = mullFork(diagnostics, "worker");

    auto start = high_resolution_clock::now();

    if (workerPID == 0) {
      freopen(stderrFilename.c_str(), "w", stderr);
      freopen(stdoutFilename.c_str(), "w", stdout);

      sharedResult->status = function();

      auto elapsed = high_resolution_clock::now() - start;
      sharedResult->runningTime = duration_cast<std::chrono::milliseconds>(elapsed).count();

      fflush(stderr);
      fflush(stdout);

      _exit(MullExitCode);
    }

    int status = 0;
    const pid_t exitedPID = waitpid(WAIT_ANY, &status, 0);

    auto elapsed = high_resolution_clock::now() - start;
    long long runningTime = duration_cast<std::chrono::milliseconds>(elapsed).count();

    if (exitedPID == timerPID) {
      /// Timer Process finished first, meaning that the worker timed out
      kill(workerPID, SIGKILL);

      sharedResult->status = Timedout;

      sharedResult->runningTime = runningTime;
    } else if (exitedPID == workerPID) {
      kill(timerPID, SIGKILL);

      sharedResult->runningTime = runningTime;

      /// Worker Process finished first
      /// Need to check whether it has signaled (crashed) or finished normally
      if (WIFSIGNALED(status)) {
        sharedResult->status = Crashed;
      }

      else if (WIFEXITED(status) && WEXITSTATUS(status) != MullExitCode) {
        sharedResult->status = AbnormalExit;
      }

      else if (WIFEXITED(status) && WEXITSTATUS(status) == MullExitCode &&
               sharedResult->status == Invalid) {
        sharedResult->status = Passed;
      }
    } else {
      llvm_unreachable("Should not reach!");
    }

    wait(nullptr);
    exit(0);
  } else {
    pid_t pid = 0;
    while ((pid = waitpid(watchdogPID, nullptr, 0)) == -1) {
    }
  }

  wait(nullptr);

  ExecutionResult result = *sharedResult;

  result.stderrOutput = readFileAndUnlink(stderrFilename.c_str());
  result.stdoutOutput = readFileAndUnlink(stdoutFilename.c_str());

  /// Check that mummap succeeds:
  /// "On success, munmap() returns 0, on failure -1, and errno is set (probably
  /// to EINVAL)." http://linux.die.net/man/2/munmap
  int munmapResult = munmap(sharedMemory, sizeof(ExecutionResult));
  (void)munmapResult;
  assert(munmapResult == 0);

  return result;
}

mull::ExecutionResult mull::NullProcessSandbox::run(Diagnostics &diagnostics,
                                                    std::function<ExecutionStatus(void)> function,
                                                    long long timeoutMilliseconds) const {
  ExecutionResult result;
  result.status = function();
  return result;
}
