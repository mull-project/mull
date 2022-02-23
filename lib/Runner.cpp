#include "mull/Runner.h"

#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"

#include <reproc++/drain.hpp>
#include <reproc++/reproc.hpp>
#include <sstream>

using namespace std::string_literals;
using namespace mull;

static std::pair<std::string, std::string> drainProcess(reproc::process &process,
                                                        bool captureOutput) {
  if (!captureOutput) {
    reproc::drain(process, reproc::sink::null, reproc::sink::null);
    return std::make_pair("", "");
  }
  std::string out;
  reproc::sink::string outSink(out);
  std::string err;
  reproc::sink::string errSink(err);
  // Ignoring errors here
  reproc::drain(process, outSink, errSink);
  return std::make_pair(out, err);
}

Runner::Runner(Diagnostics &diagnostics) : diagnostics(diagnostics) {}

ExecutionResult Runner::runProgram(const std::string &program,
                                   const std::vector<std::string> &arguments,
                                   const std::unordered_map<std::string, std::string> &environment,
                                   long long int timeout, bool captureOutput, bool failSilently,
                                   std::optional<std::string> optionalWorkingDirectory) {
  reproc::options options;
  options.env.extra = reproc::env(environment);
  options.redirect.err.type = reproc::redirect::type::pipe;
  options.stop.first.action = reproc::stop::kill;
  options.stop.first.timeout = std::chrono::milliseconds(100);
  options.deadline = std::chrono::milliseconds(timeout);
  if (auto &workingDirectory = optionalWorkingDirectory) {
    options.working_directory = workingDirectory->c_str();
  }

  std::vector<std::string> allArguments{ program };
  std::copy(std::begin(arguments), std::end(arguments), std::back_inserter(allArguments));
  auto start = std::chrono::high_resolution_clock::now();

  reproc::process process;
  std::error_code ec = process.start(allArguments, options);
  if (!failSilently) {
    if (ec == std::errc::no_such_file_or_directory) {
      diagnostics.error("Executable not found: "s + program);
    } else if (ec) {
      std::stringstream errorMessage;
      errorMessage << "Cannot run executable: " << ec.message() << '\n';
      auto outputs = drainProcess(process, true);
      errorMessage << "stdout: " << outputs.first << '\n';
      errorMessage << "stderr: " << outputs.second << '\n';
      errorMessage << "command: ";
      for (auto &arg : allArguments) {
        errorMessage << arg << " ";
      }
      errorMessage << '\n';
      diagnostics.error(errorMessage.str());
    }
  }

  int status;

  auto outputs = drainProcess(process, captureOutput);
  std::tie(status, ec) = process.wait(reproc::milliseconds(timeout));
  ExecutionStatus executionStatus = Failed;
  if (ec == std::errc::timed_out) {
    process.kill();
    executionStatus = Timedout;
  } else if (status == 0) {
    executionStatus = Passed;
  } else {
    executionStatus = Failed;
  }

  auto elapsed = std::chrono::high_resolution_clock::now() - start;
  ExecutionResult result;
  result.runningTime = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
  result.exitStatus = status;
  result.stdoutOutput = outputs.first;
  result.stderrOutput = outputs.second;
  result.status = executionStatus;

  return result;
}
