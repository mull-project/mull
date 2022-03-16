#include "mull/Reporters/PatchesReporter.h"

#include "mull/Bitcode.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/ExecutionResult.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Reporters/SourceCodeReader.h"
#include "mull/Result.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>

#include <fstream>
#include <numeric>
#include <regex>
#include <sstream>
#include <string>
#include <unistd.h>
#include <utility>

using namespace mull;
using namespace llvm;
using namespace std::string_literals;

static std::string getReportName(const std::string &name) {
  std::string reportName = name;
  if (reportName.empty()) {
    time_t t;
    time(&t);
    reportName = std::to_string(t);
  }
  return reportName + "-patches";
}

static std::string getReportDir(const std::string &reportDir) {
  if (reportDir.empty()) {
    return std::string(".");
  }
  return reportDir;
}

PatchesReporter::PatchesReporter(Diagnostics &diagnostics, const std::string &reportDir,
                                 const std::string &reportName, const std::string &basePath,
                                 std::unordered_map<std::string, std::string> mullInformation)
    : diagnostics(diagnostics),
      patchesPath(getReportDir(reportDir) + "/" + getReportName(reportName)),
      basePathRegex("^" + getReportDir(basePath)), sourceCodeReader(),
      mullInformation(std::move(mullInformation)) {
  llvm::sys::fs::create_directories(patchesPath, true);
}

std::string mull::PatchesReporter::getPatchesPath() {
  return patchesPath;
}

void mull::PatchesReporter::reportResults(const Result &result) {
  MutatorsFactory factory(diagnostics);
  factory.init();

  std::string mullInfo;
  {
    std::stringstream mullInfoStream;
    std::vector<std::string> keys;
    for (auto &info : mullInformation) {
      keys.push_back(info.first);
    }
    std::sort(std::begin(keys), std::end(keys));
    for (auto &key : keys) {
      mullInfoStream << key << ": " << mullInformation[key] << "\n";
    }
    mullInfo = mullInfoStream.str();
  }

  for (auto &mutationResult : result.getMutationResults()) {

    const ExecutionResult mutationExecutionResult = mutationResult->getExecutionResult();

    const auto mutant = *mutationResult->getMutant();
    const auto &sourceLocation = mutant.getSourceLocation();
    if (sourceLocation.isNull() || !sourceLocation.canRead()) {
      diagnostics.warning("PatchesReporter: Cannot report '"s + mutant.getIdentifier() +
                          "': cannot read "s + sourceLocation.filePath);
      return;
    }
    const auto &sourceEndLocation = mutant.getEndLocation();
    const std::string sourceBasename =
        std::regex_replace(sourceLocation.filePath.substr(sourceLocation.directory.size() + 1),
                           std::regex("([/]|\\.(?!patch))"),
                           "_");

    const auto mutator = factory.getMutator(mutant.getMutatorIdentifier());
    const std::vector<std::string> sourceLines =
        sourceCodeReader.getSourceLines(sourceLocation, sourceEndLocation);
    const std::string sourcePath = std::regex_replace(sourceLocation.filePath, basePathRegex, "");

    const std::string prefix = [&mutationExecutionResult]() {
      switch (mutationExecutionResult.status) {
      case ExecutionStatus::Passed:
        return "survived-";
        break;
      case ExecutionStatus::NotCovered:
        return "uncovered-";
        break;
      default:
        return "killed-";
      }
    }();

    const std::string filename = [&]() {
      std::stringstream filenamebuilder;
      filenamebuilder << patchesPath << "/" << prefix << sourceBasename << "-"
                      << mutant.getMutatorIdentifier() << "-L" << sourceLocation.line << "-C"
                      << sourceLocation.column << ".patch";
      return filenamebuilder.str();
    }();

    diagnostics.debug("Writing Patchfile: "s + filename);
    const size_t lines = sourceEndLocation.line - sourceLocation.line + 1;
    std::ofstream myfile{ filename };
    myfile << "--- a" << (sourcePath[0] != '/' ? "/" : "") << sourcePath << " 0"
           << "\n"
           << "+++ b" << (sourcePath[0] != '/' ? "/" : "") << sourcePath << " 0"
           << "\n"
           << "@@ -" << sourceLocation.line << "," << lines << " +" << sourceLocation.line
           << ",1 @@\n";
    for (auto &currentLine : sourceLines) {
      myfile << "-" << currentLine;
    }
    myfile << "+" << sourceLines.front().substr(0, sourceLocation.column - 1)
           << mutator->getReplacement() << sourceLines.back().substr(sourceEndLocation.column - 1);
    myfile << "--\n" << mullInfo;
    myfile.flush();
    if (!myfile.good()) {
      diagnostics.warning("Writing Patchfile failed"s + filename);
    }
    myfile.close();
  }

  diagnostics.info("Patchfiles can be found at '"s + patchesPath + "'");
}
