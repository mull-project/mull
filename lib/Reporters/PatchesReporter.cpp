#include "mull/Reporters/PatchesReporter.h"

#include "mull/Bitcode.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/ExecutionResult.h"
#include "mull/Mutant.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Reporters/SourceCodeReader.h"
#include "mull/Result.h"
#include "mull/SourceLocation.h"

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

using namespace mull;
using namespace llvm;

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

PatchesReporter::PatchesReporter(
    Diagnostics &diagnostics, const std::string &reportDir, const std::string &reportName,
    const std::string basePath, const std::unordered_map<std::string, std::string> &mullInformation)
    : diagnostics(diagnostics),
      patchesPath(getReportDir(reportDir) + "/" + getReportName(reportName)),
      basePathRegex("^" + getReportDir(basePath)), sourceCodeReader(),
      mullInformation(mullInformation) {
  llvm::sys::fs::create_directories(patchesPath, true);
}

std::string mull::PatchesReporter::getPatchesPath() {
  return patchesPath;
}

void mull::PatchesReporter::reportResults(const Result &result) {
  MutatorsFactory factory(diagnostics);
  factory.init();
  for (auto &mutationResult : result.getMutationResults()) {

    const ExecutionResult mutationExecutionResult = mutationResult->getExecutionResult();

    const auto mutant = *mutationResult->getMutant();
    const auto &sourceLocation = mutant.getSourceLocation();
    const auto &sourceEndLocation = mutant.getEndLocation();
    const std::string sourceBasename =
        std::regex_replace(sourceLocation.filePath.substr(sourceLocation.directory.size() + 1),
                           std::regex("([/]|\\.(?!patch))"),
                           "_");
    ;
    const auto mutator = factory.getMutator(mutant.getMutatorIdentifier());
    const std::string sourceLine = sourceCodeReader.getSourceLine(sourceLocation);
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

    diagnostics.debug(std::string("Writing Patchfile: ") + filename.c_str());
    std::ofstream myfile{ filename };
    myfile << "--- a" << sourcePath << " 0"
           << "\n"
           << "+++ b" << sourcePath << " 0"
           << "\n"
           << "@@ -" << sourceLocation.line << ",1 +" << sourceLocation.line << ",1 @@\n"
           << "-" << sourceLine << "+" << sourceLine.substr(0, sourceLocation.column - 1)
           << mutator->getReplacement() << sourceLine.substr(sourceEndLocation.column - 1);
    myfile << std::accumulate(
        mullInformation.begin(),
        mullInformation.end(),
        std::string("--\n"),
        [](std::string in, auto &s) { return std::move(in) + s.first + ": " + s.second + "\n"; });

    myfile.flush();
    if (!myfile.good())
      diagnostics.warning(std::string("Writing Patchfile failed") + filename.c_str());
    myfile.close();
  }

  diagnostics.info(std::string("Patchfiles can be found at '") + patchesPath + "'");
}
