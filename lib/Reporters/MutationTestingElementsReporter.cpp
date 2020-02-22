#include "mull/Reporters/MutationTestingElementsReporter.h"

#include "mull/Diagnostics/Diagnostics.h"
#include "mull/MutationResult.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Reporters/ASTSourceInfoProvider.h"
#include "mull/Reporters/SourceManager.h"
#include "mull/Result.h"

#include <json11/json11.hpp>
#include <llvm/Support/FileSystem.h>

#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace mull;
using namespace json11;

static bool mutantSurvived(const ExecutionStatus &status) {
  return status == ExecutionStatus::Passed;
}

static json11::Json createFiles(Diagnostics &diagnostics, const Result &result,
                                const std::set<MutationPoint *> &killedMutants,
                                SourceInfoProvider &sourceInfoProvider) {
  SourceManager sourceManager;

  Json::object filesJSON;

  // Mutation Testing Elements Schema suggests storing mutation points based
  // on their source file.
  // Step 1: we create a map: "file => all of its mutation points".
  std::map<std::string, std::vector<MutationPoint *>> mutationPointsPerFile;
  for (auto mutant : result.getMutationPoints()) {
    auto &sourceLocation = mutant->getSourceLocation();
    auto sourceCodeLine = sourceManager.getLine(sourceLocation);
    assert(sourceLocation.column < sourceCodeLine.size());

    mutationPointsPerFile[sourceLocation.filePath].push_back(mutant);
  }

  // Step 2: Iterate through each file and dump the information about each
  // mutation points to its file's JSON entry.
  for (auto &fileMutationPoints : mutationPointsPerFile) {
    Json::object fileJSON;
    fileJSON["language"] = "cpp";

    // Getting source code
    std::ifstream fileInputStream(fileMutationPoints.first);
    std::stringstream sourceCodeBuffer;
    sourceCodeBuffer << fileInputStream.rdbuf();
    fileJSON["source"] = sourceCodeBuffer.str();

    Json::array mutantsEntries;

    for (MutationPoint *mutationPoint : fileMutationPoints.second) {
      MutationPointSourceInfo sourceInfo =
          sourceInfoProvider.getSourceInfo(diagnostics, mutationPoint);

      std::string status = (killedMutants.count(mutationPoint) == 0) ? "Survived" : "Killed";

      Json mpJson =
          Json::object{ { "id", mutationPoint->getMutator()->getUniqueIdentifier() },
                        { "mutatorName", mutationPoint->getDiagnostics() },
                        { "replacement", mutationPoint->getReplacement() },
                        { "location",
                          Json::object{ { "start",
                                          Json::object{ { "line", sourceInfo.beginLine },
                                                        { "column", sourceInfo.beginColumn } } },
                                        { "end",
                                          Json::object{ { "line", sourceInfo.endLine },
                                                        { "column", sourceInfo.endColumn } } } } },
                        { "status", status } };
      mutantsEntries.push_back(mpJson);
    }

    fileJSON["mutants"] = mutantsEntries;
    filesJSON[fileMutationPoints.first] = fileJSON;
  }

  return filesJSON;
}

static std::string getFilename(const std::string &name) {
  if (name.empty()) {
    time_t t;
    time(&t);
    return std::to_string(t);
  }
  return name;
}

static std::string getReportDir(const std::string &dir) {
  if (dir.empty()) {
    return std::string(".");
  }
  return dir;
}

MutationTestingElementsReporter::MutationTestingElementsReporter(
    Diagnostics &diagnostics, const std::string &reportDir, const std::string &reportName,
    SourceInfoProvider &sourceInfoProvider)
    : diagnostics(diagnostics), filename(getFilename(reportName)),
      htmlPath(getReportDir(reportDir) + "/" + filename + ".html"),
      jsonPath(getReportDir(reportDir) + "/" + filename + ".json"),
      sourceInfoProvider(sourceInfoProvider) {
  llvm::sys::fs::create_directories(reportDir);
}

void MutationTestingElementsReporter::reportResults(const Result &result, const Metrics &metrics) {
  if (result.getMutationPoints().empty()) {
    diagnostics.info("No mutants found. Mutation score: infinitely high");
    return;
  }
  generateHTMLFile();

  std::set<MutationPoint *> killedMutants;
  for (auto &mutationResult : result.getMutationResults()) {
    auto mutant = mutationResult->getMutationPoint();
    auto &executionResult = mutationResult->getExecutionResult();

    if (!mutantSurvived(executionResult.status)) {
      killedMutants.insert(mutant);
    }
  }

  auto rawScore = double(killedMutants.size()) / double(result.getMutationPoints().size());
  auto score = uint(rawScore * 100);

  Json json = Json::object{
    { "mutationScore", (int)score },
    { "thresholds", Json::object{ { "high", 80 }, { "low", 60 } } },
    { "files", createFiles(diagnostics, result, killedMutants, sourceInfoProvider) },
  };
  std::string json_str = json.dump();

  diagnostics.info(std::string("Mutation Testing Elements reporter: generating report to ") +
                   jsonPath);

  std::ofstream out(jsonPath);
  out << json_str;
  out.close();
}
const std::string &MutationTestingElementsReporter::getJSONPath() {
  return jsonPath;
}

void MutationTestingElementsReporter::generateHTMLFile() {
  const char *url = "https://www.unpkg.com/mutation-testing-elements";

  std::ofstream out(htmlPath);
  out << "<!DOCTYPE html>\n";
  out << "<head>\n";
  out << "  <title>Mutation Testing Elements</title>\n";
  out << "  <script defer src=\"" << url << "\"></script>";
  out << "</head>\n";
  out << "<body>\n";
  out << "  <mutation-test-report-app src=\"" << filename << ".json\""
      << "</mutation-test-report-app>\n";
  out << "</body>\n";
  out << "</html>";

  out.close();
}
