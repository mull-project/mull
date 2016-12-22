
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
  const std::string fileName = instruction->getDebugLoc()->getFilename();
  const std::string line = std::to_string(instruction->getDebugLoc()->getLine());

  ///Function *function = instruction->getFunction();
  ///assert(function && "Expected function");
  ///std::string functionName = function->getName().str();

  std::stringstream mpComponentAsStream;
  mpComponentAsStream << fileName;
  mpComponentAsStream << ":";
  mpComponentAsStream << line;
  callerPath.push_back(mpComponentAsStream.str());

  // The following loop stops on test as it does not have a caller.
  do {
    Instruction *instruction = currentTestee->getCallerInstruction();

    const std::string fileName = instruction->getDebugLoc()->getFilename();
    const std::string line = std::to_string(instruction->getDebugLoc()->getLine());

    std::stringstream callerComponentAsStream;
    callerComponentAsStream << fileName;
    callerComponentAsStream << ":";
    callerComponentAsStream << line;
    callerPath.push_back(callerComponentAsStream.str());

    currentTestee = currentTestee->getCallerTestee();
  } while (currentTestee->getCallerTestee() != nullptr);

  std::reverse(std::begin(callerPath), std::end(callerPath));

  return callerPath;
}
