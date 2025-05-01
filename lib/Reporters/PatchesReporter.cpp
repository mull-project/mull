#include "mull/Reporters/PatchesReporter.h"

#include "mull/Bitcode.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/ExecutionResult.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Reporters/SourceCodeReader.h"
#include "mull/Result.h"

#include <llvm/ADT/StringExtras.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/SHA256.h>

#include <climits>
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

static std::string patchFilename(const std::string patchesPath, const Mutant &mutant,
                                 const ExecutionResult &executionResult) {
  const std::string prefix = [&executionResult]() {
    switch (executionResult.status) {
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
  auto sourceLocation = mutant.getSourceLocation();
  const std::string sourceBasename =
      std::regex_replace(sourceLocation.filePath.substr(sourceLocation.directory.size() + 1),
                         std::regex("([/]|\\.(?!patch))"),
                         "_");
  {
    std::stringstream ss;
    ss << prefix << sourceBasename << "-" << mutant.getMutatorIdentifier() << "-L"
       << sourceLocation.line << "-C" << sourceLocation.column << ".patch";
    auto s = ss.str();
    if (s.size() < NAME_MAX) {
      return patchesPath + "/" + s;
    }
  }
  {
    std::stringstream ss;
    ss << prefix << mutant.getMutatorIdentifier() << "-L" << sourceLocation.line << "-C"
       << sourceLocation.column << ".patch";
    auto s = ss.str();
    if (s.size() < NAME_MAX) {
      return patchesPath + "/" + s;
    }
  }
  {
    llvm::SHA256 sha;
    sha.update(sourceBasename);
    sha.update(mutant.getMutatorIdentifier());
    std::stringstream ss;
    ss << prefix << llvm::toHex(sha.final()) << "-L" << sourceLocation.line << "-C"
       << sourceLocation.column << ".patch";
    return patchesPath + "/" + ss.str();
  }
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

    const auto mutator = factory.getMutator(mutant.getMutatorIdentifier());
    const std::vector<std::string> sourceLines =
        sourceCodeReader.getSourceLines(sourceLocation, sourceEndLocation);
    const std::string sourcePath = std::regex_replace(sourceLocation.filePath, basePathRegex, "");

    const std::string filename = patchFilename(patchesPath, mutant, mutationExecutionResult);

    diagnostics.debug("Writing Patchfile: "s + filename);
    const size_t lines = sourceEndLocation.line - sourceLocation.line + 1;
    std::error_code ec;
    llvm::raw_fd_ostream output(filename, ec);
    if (ec) {
      diagnostics.warning("Cannot create patchfile: "s + std::to_string(ec.value()));
      diagnostics.warning("Cannot create patchfile: "s + filename + ": " + ec.message());
      return;
    }
    output << "--- a" << (sourcePath[0] != '/' ? "/" : "") << sourcePath << " 0"
           << "\n"
           << "+++ b" << (sourcePath[0] != '/' ? "/" : "") << sourcePath << " 0"
           << "\n"
           << "@@ -" << sourceLocation.line << "," << lines << " +" << sourceLocation.line
           << ",1 @@\n";
    for (auto &currentLine : sourceLines) {
      output << "-" << currentLine;
    }
    output << "+" << sourceLines.front().substr(0, sourceLocation.column - 1)
           << mutator->getReplacement() << sourceLines.back().substr(sourceEndLocation.column - 1);
    output << "--\n" << mullInfo;
    output.flush();
    output.close();
  }

  diagnostics.info("Patchfiles can be found at '"s + patchesPath + "'");
}
