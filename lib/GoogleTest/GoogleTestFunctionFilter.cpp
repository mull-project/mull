#include "GoogleTest/GoogleTestFunctionFilter.h"

#include <llvm/IR/DebugInfoMetadata.h>

using namespace llvm;
using namespace mull;

bool GoogleTestFunctionFilter::shouldSkipFunction(llvm::Function *function) {
  if (function->getName().find(StringRef("testing8internal")) != StringRef::npos) {
    return true;
  }

  if (function->getName().find(StringRef("testing15AssertionResult")) != StringRef::npos) {
    return true;
  }

  if (function->getName().find(StringRef("testing7Message")) != StringRef::npos) {
    return true;
  }

  if (function->getName().find(StringRef("clang_call_terminate")) != StringRef::npos) {
    return true;
  }

  if (function->hasMetadata()) {
    int debugInfoKindID = 0;
    MDNode *debug = function->getMetadata(debugInfoKindID);
    DISubprogram *subprogram = dyn_cast<DISubprogram>(debug);
    if (subprogram) {
      if (subprogram->getFilename().str().find("include/c++/v1") != std::string::npos) {
        return true;
      }

      if (subprogram->getFilename().str().find("gmock") != std::string::npos) {
        return true;
      }

      if (subprogram->getFilename().str().find("gtest") != std::string::npos) {
        return true;
      }
    }
  }

  return false;
}
