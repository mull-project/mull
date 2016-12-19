
#include "Result.h"

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"

#include <string>

using namespace Mutang;
using namespace llvm;

std::string Result::printPath(MutationResult *mutationResult) {
  printf("Result::printPath\n");

  Testee *currentTestee = mutationResult->getTestee();

  char formatBuffer[256];

  std::vector<std::string> strings;

  /// Last path component: mutation point itself.
  auto mutationPoint = mutationResult->getMutationPoint();
  Instruction *instruction = dyn_cast<Instruction>(mutationPoint->getOriginalValue());
  std::string fileName = instruction->getDebugLoc()->getFilename();
  std::string line = std::to_string(instruction->getDebugLoc()->getLine());
  Function *function = instruction->getFunction();
  assert(function && "Expected function");

  std::string functionName = function->getName().str();

  snprintf(formatBuffer, sizeof(formatBuffer), "-- %s:%s, %s\n", fileName.c_str(), line.c_str(), functionName.c_str());
  std::string testeeLine(formatBuffer);
  strings.push_back(testeeLine);

  // The following loop stops on test as it does not have a caller.
  do {
    Instruction *instruction = currentTestee->getCallerInstruction();

    std::string fileName = instruction->getDebugLoc()->getFilename();
    std::string line = std::to_string(instruction->getDebugLoc()->getLine());

    Function *function = nullptr;
    if ((function = instruction->getFunction())) {
      std::string functionName = function->getName().str();

      snprintf(formatBuffer, sizeof(formatBuffer), "-- %s:%s, %s\n", fileName.c_str(), line.c_str(), functionName.c_str());

      std::string testeeLine(formatBuffer);
      strings.push_back(testeeLine);
    }

    currentTestee = currentTestee->getCallerTestee();
  } while (currentTestee->getCallerTestee() != nullptr);

  std::string result;

  for (int i = strings.size() - 1, counter = 0; i >= 0 ; --i, counter++) {
    result.insert(result.size(), counter * 2, ' ');
    result.append(strings[i]);
  }

  printf("caller path:\n%s", result.c_str());

  return result;
}
