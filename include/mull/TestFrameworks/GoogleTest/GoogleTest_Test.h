#pragma once

#include "mull/TestFrameworks/Test.h"

#include <vector>

namespace llvm {

class Function;

}

namespace mull {

class GoogleTest_Test : public Test {
  std::string TestName;
  llvm::Function *TestBodyFunction;

public:
  GoogleTest_Test(std::string Name, llvm::Function *TestBody);

  std::string getTestName() override;
  std::string getTestDisplayName() override;

  std::string getUniqueIdentifier() override { return getTestName(); };

  llvm::Function *GetTestBodyFunction();

  std::vector<llvm::Function *> entryPoints() override {
    return std::vector<llvm::Function *>({TestBodyFunction});
  }

  llvm::Function *testBodyFunction() override { return TestBodyFunction; }

  static bool classof(const Test *T) { return T->getKind() == TK_GoogleTest; }
};

} // namespace mull
