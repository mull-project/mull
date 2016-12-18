
#include "Result.h"

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"

using namespace Mutang;
using namespace llvm;

void Result::printPath(Testee *testee) {
  printf("Result::printPath\n");
//
//  + "'" + instruction->getDebugLoc()->getFilename().str() + "',"
//  + "'" + std::to_string(instruction->getDebugLoc()->getLine()) + "',"
//  + "'" + std::to_string(instruction->getDebugLoc()->getColumn()) + "',"

  Testee *testeeIter = testee;

  while (testeeIter != nullptr) {
    Function *testeeFunction = testeeIter->getTesteeFunction();

    int debugInfoKindID = 0;
    MDNode *debug = testeeFunction->getMetadata(debugInfoKindID);

    DISubprogram *sp = getDISubprogram(debug);

    StringRef functionName = testeeFunction->getName();
    StringRef filename = sp->getFilename();
    unsigned line = sp->getLine();

    printf("%s:%u %s\n", filename.str().c_str(), line, functionName.str().c_str());

    testeeIter = testeeIter->getCallerTestee();
  }

#warning WIP
  exit(1);
}
