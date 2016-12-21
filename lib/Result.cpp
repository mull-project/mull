
#include "Result.h"

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"

#include <sstream>
#include <string>

using namespace Mutang;
using namespace llvm;

std::vector<std::string> Result::calculateCallerPath(MutationResult *mutationResult) {
  Testee *currentTestee = mutationResult->getTestee();

  std::vector<std::string> callerPath;

  /// Currently we don't print functions, only "file:line". Later we will and
  /// for that we will have to demangle them.

  /// Last path component: mutation point itself.
  auto mutationPoint = mutationResult->getMutationPoint();
  Instruction *instruction = dyn_cast<Instruction>(mutationPoint->getOriginalValue());
  std::string fileName = instruction->getDebugLoc()->getFilename();
  std::string line = std::to_string(instruction->getDebugLoc()->getLine());

  ///Function *function = instruction->getFunction();
  ///assert(function && "Expected function");
  ///std::string functionName = function->getName().str();

  std::stringstream mpComponentAsStream;
  mpComponentAsStream << fileName.c_str();
  mpComponentAsStream << ":";
  mpComponentAsStream << line.c_str();
  std::string mpComponent = mpComponentAsStream.str();

  callerPath.push_back(mpComponent);

  // The following loop stops on test as it does not have a caller.
  do {
    Instruction *instruction = currentTestee->getCallerInstruction();

    std::string fileName = instruction->getDebugLoc()->getFilename();
    std::string line = std::to_string(instruction->getDebugLoc()->getLine());

    std::stringstream callerComponentAsStream;
    callerComponentAsStream << fileName.c_str();
    callerComponentAsStream << ":";
    callerComponentAsStream << line.c_str();
    std::string callerComponent = callerComponentAsStream.str();

    callerPath.push_back(callerComponent);

    currentTestee = currentTestee->getCallerTestee();
  } while (currentTestee->getCallerTestee() != nullptr);

  std::reverse(std::begin(callerPath), std::end(callerPath));

  return callerPath;
}
