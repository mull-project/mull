#include "Filter.h"
#include "TestFrameworks/Test.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include "SourceLocation.h"

using namespace llvm;
using namespace mull;

bool Filter::shouldSkipInstruction(llvm::Instruction *instruction) {
  SourceLocation location = SourceLocation::sourceLocationFromInstruction(instruction);
  if (location.isNull()) {
    return false;
  }

  for (auto &fileLocation : locations) {
    if (location.filePath.find(fileLocation) != std::string::npos) {
      return true;
    }
  }

  return false;
}

bool Filter::shouldSkipFunction(llvm::Function *function) {
  for (std::string &name : names) {
    if (function->getName().find(StringRef(name)) != StringRef::npos) {
      return true;
    }
  }

  SourceLocation location = SourceLocation::sourceLocationFromFunction(function);
  if (location.isNull()) {
    return false;
  }

  for (auto &fileLocation : locations) {
    if (location.filePath.find(fileLocation) != std::string::npos) {
      return true;
    }
  }

  return false;
}

bool Filter::shouldSkipTest(const std::string &testName) {
  if (tests.empty()) {
    return false;
  }

  auto iterator = std::find(tests.begin(), tests.end(), testName);
  if (iterator != tests.end()) {
    return false;
  }

  return true;
}

void Filter::skipByName(const std::string &nameSubstring) {
  names.push_back(nameSubstring);
}

void Filter::skipByName(const char *nameSubstring) {
  names.push_back(std::string(nameSubstring));
}

void Filter::skipByLocation(const std::string &locationSubstring) {
  locations.push_back(locationSubstring);
}

void Filter::skipByLocation(const char *locationSubstring) {
  locations.push_back(std::string(locationSubstring));
}

void Filter::includeTest(const std::string &testName) {
  tests.push_back(testName);
}

void Filter::includeTest(const char *testName) {
  tests.push_back(std::string(testName));
}
