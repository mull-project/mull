#pragma once

#include <string>
#include <vector>

namespace llvm {
class Function;
}

namespace mull {

class Test {
public:
  virtual std::string getTestName() = 0;
  virtual std::string getTestDisplayName() = 0;
  virtual std::string getUniqueIdentifier() = 0;
  virtual ~Test() {}
  /// Entry points into the test might be the test body, setup/teardown,
  /// before each/before all functions, and so on.
  /// TODO: entryPoints is not the best name for teardown/after each methods
  virtual std::vector<llvm::Function *> entryPoints() {
    return std::vector<llvm::Function *>();
  }

  enum TestKind {
    TK_SimpleTest,
    TK_GoogleTest,
    TK_RustTest
  };
  TestKind getKind() const { return Kind; }
  Test(TestKind K) : Kind(K) {}
private:
  const TestKind Kind;
};

}
