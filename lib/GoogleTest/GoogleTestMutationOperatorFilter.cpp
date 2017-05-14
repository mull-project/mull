#include "GoogleTestMutationOperatorFilter.h"

#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;
using namespace mull;

GoogleTestMutationOperatorFilter::GoogleTestMutationOperatorFilter(
  std::vector<std::string> testsToFilter) :
  testsToFilter(testsToFilter) {}

bool GoogleTestMutationOperatorFilter::shouldSkipDefinedFunction(llvm::Function *definedFunction) {
  if (definedFunction->getName().find(StringRef("testing8internal")) != StringRef::npos) {
    return true;
  }

  if (definedFunction->getName().find(StringRef("testing15AssertionResult")) != StringRef::npos) {
    return true;
  }

  if (definedFunction->getName().find(StringRef("testing7Message")) != StringRef::npos) {
    return true;
  }

  if (definedFunction->hasMetadata()) {
    int debugInfoKindID = 0;
    MDNode *debug = definedFunction->getMetadata(debugInfoKindID);
    DISubprogram *subprogram = dyn_cast<DISubprogram>(debug);
    if (subprogram) {
      if (subprogram->getFilename().str().find("include/c++/v1") != std::string::npos) {
        return true;
      }
    }
  }

  return false;
}

bool GoogleTestMutationOperatorFilter::shouldSkipTest(const std::string &testName) {
  if (testsToFilter.empty()) {
    return false;
  }

  if (std::find(testsToFilter.begin(),
                testsToFilter.end(),
                testName) == testsToFilter.end()) {
    return true;
  }

  return false;
}

bool GoogleTestMutationOperatorFilter::shouldSkipInstruction(llvm::Instruction *instruction) {
  if (instruction->hasMetadata()) {
    int debugInfoKindID = 0;
    MDNode *debug = instruction->getMetadata(debugInfoKindID);

    DILocation *location = dyn_cast<DILocation>(debug);
    if (location) {
      if (location->getFilename().str().find("include/c++/v1") != std::string::npos) {
        return true;
      }
    }
  }

  return false;
};
