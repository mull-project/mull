#include "mull/Toolchain/Linker.h"

#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Toolchain/Runner.h"

#include <llvm/Support/FileSystem.h>
#include <sstream>

using namespace mull;
using namespace std::string_literals;

std::string outputFileName(const Configuration &configuration, Diagnostics &diagnostics) {
  if (configuration.outputFile.empty()) {
    llvm::Twine prefix("mull");
    llvm::SmallString<128> resultPath;

    if (std::error_code err = llvm::sys::fs::createTemporaryFile(prefix, "exe", resultPath)) {
      diagnostics.error("Cannot create temporary file"s + err.message());
      return std::string();
    }
    return resultPath.str().str();
  }
  return configuration.outputFile;
}

Linker::Linker(const Configuration &configuration, Diagnostics &diagnostics)
    : configuration(configuration), diagnostics(diagnostics) {}

std::string Linker::linkObjectFiles(const std::vector<std::string> &objects) {
  std::string outputFile = outputFileName(configuration, diagnostics);
  Runner runner(diagnostics);
  std::vector<std::string> arguments;
  std::copy(std::begin(configuration.linkerFlags),
            std::end(configuration.linkerFlags),
            std::back_inserter(arguments));
  std::copy(std::begin(objects), std::end(objects), std::back_inserter(arguments));
  arguments.emplace_back("-o");
  arguments.push_back(outputFile);
  ExecutionResult result = runner.runProgram(
      configuration.linker, arguments, {}, configuration.linkerTimeout, true, std::nullopt);
  std::stringstream commandStream;
  commandStream << configuration.linker;
  for (std::string &argument : arguments) {
    commandStream << ' ' << argument;
  }
  std::string command = commandStream.str();
  if (result.status != Passed) {
    std::stringstream message;
    message << "Cannot link program\n";
    message << "status: " << result.getStatusAsString() << "\n";
    message << "time: " << result.runningTime << "ms\n";
    message << "exit: " << result.exitStatus << "\n";
    message << "command: " << command << "\n";
    message << "stdout: " << result.stdoutOutput << "\n";
    message << "stderr: " << result.stderrOutput << "\n";
    diagnostics.error(message.str());
  }
  diagnostics.debug("Link command: "s + command);
  return outputFile;
}
