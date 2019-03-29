#include "Reporters/IDEReporter.h"

#include "Logger.h"
#include "MutationResult.h"
#include "Result.h"

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace mull;

struct LineOffset {
  FILE *file;
  std::vector<uint> offsets;
  LineOffset(FILE *file, std::vector<uint> offsets)
      : file(file), offsets(std::move(offsets)) {}
};

class SourceManager {
public:
  std::string getLine(const SourceLocation &location) {
    LineOffset &lineOffset = getLineOffset(location);
    assert(location.line < lineOffset.offsets.size());

    uint lineBegin = lineOffset.offsets[location.line - 1];
    uint lineEnd = lineOffset.offsets[location.line];
    uint lineLength = lineEnd - lineBegin;
    fseek(lineOffset.file, lineBegin, SEEK_SET);
    char *buffer = new char[lineLength + 1];
    fread(buffer, sizeof(char), lineLength, lineOffset.file);
    buffer[lineLength] = '\0';
    auto line = std::string(buffer);
    free(buffer);
    return line;
  }
  ~SourceManager() {
    for (auto &pair : lineOffsets) {
      fclose(pair.second.file);
    }
  }

private:
  std::map<std::string, LineOffset> lineOffsets;

  LineOffset &getLineOffset(const SourceLocation &location) {
    if (lineOffsets.count(location.filePath)) {
      return lineOffsets.at(location.filePath);
    }

    FILE *file = fopen(location.filePath.c_str(), "rb");
    if (!file) {
      perror("IDEReporter");
    }
    std::vector<uint> offsets;
    uint offset = 0;
    char c = '\n';
    for (; c != EOF; c = fgetc(file), offset++) {
      if (c == '\n') {
        offsets.push_back(offset);
      }
    }

    LineOffset lineOffset(file, offsets);
    auto inserted =
        lineOffsets.insert(std::make_pair(location.filePath, lineOffset));
    return inserted.first->second;
  }
};

static bool mutantSurvived(const ExecutionStatus &status) {
  return status == ExecutionStatus::Passed;
}

static void printSurvivedMutant(SourceManager &sourceManager,
                                const MutationPoint &mutant) {
  auto &sourceLocation = mutant.getSourceLocation();
  Logger::info() << sourceLocation.filePath << ":" << sourceLocation.line << ":"
                 << sourceLocation.column
                 << ": warning: " << mutant.getDiagnostics() << "\n";
  auto line = sourceManager.getLine(sourceLocation);
  assert(sourceLocation.column < line.size());
  std::string caret(sourceLocation.column, ' ');
  for (size_t index = 0; index < sourceLocation.column; index++) {
    if (line[index] == '\t') {
      caret[index] = '\t';
    }
  }
  caret[sourceLocation.column - 1] = '^';

  Logger::info() << line;
  Logger::info() << caret << "\n";
}

void IDEReporter::reportResults(const Result &result, const RawConfig &config,
                                const Metrics &metrics) {
  if (result.getMutationPoints().empty()) {
    Logger::info() << "No mutants found. Mutation score: infinitely high\n";
    return;
  }

  std::set<MutationPoint *> killedMutants;
  for (auto &mutationResult : result.getMutationResults()) {
    auto mutant = mutationResult->getMutationPoint();
    auto &executionResult = mutationResult->getExecutionResult();

    if (!mutantSurvived(executionResult.status)) {
      killedMutants.insert(mutant);
    }
  }

  auto survivedMutantsCount =
      result.getMutationPoints().size() - killedMutants.size();

  Logger::info() << "\nSurvived mutants (" << survivedMutantsCount << "/"
                 << result.getMutationPoints().size() << "):\n\n";

  SourceManager sourceManager;
  for (auto mutant : result.getMutationPoints()) {
    if (killedMutants.find(mutant) == killedMutants.end()) {
      printSurvivedMutant(sourceManager, *mutant);
    }
  }

  auto rawScore =
      double(killedMutants.size()) / double(result.getMutationPoints().size());
  auto score = uint(rawScore * 100);
  Logger::info() << "Mutation score: " << score << "%\n";
}
