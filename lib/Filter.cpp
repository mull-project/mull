#include "Filter.h"
#include "Test.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>

using namespace llvm;
using namespace mull;

bool Filter::shouldSkipInstruction(llvm::Instruction *instruction) {
  if (instruction->hasMetadata()) {
    int debugInfoKindID = 0;
    MDNode *debug = instruction->getMetadata(debugInfoKindID);

    DILocation *location = dyn_cast<DILocation>(debug);
    if (location) {
      for (std::string &fileLocation : locations) {
        if (location->getFilename().str().find(fileLocation) != std::string::npos) {
          return true;
        }
      }
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

  if (function->hasMetadata()) {
    int debugInfoKindID = 0;
    MDNode *debug = function->getMetadata(debugInfoKindID);
    DISubprogram *subprogram = dyn_cast<DISubprogram>(debug);
    if (subprogram) {
      for (std::string &location : locations) {
        if (subprogram->getFilename().str().find(location) != std::string::npos) {
          return true;
        }
      }
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
