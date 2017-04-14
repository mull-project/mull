#pragma once

#include <string>

namespace mull {

class Test {
public:
  virtual std::string getTestName() = 0;
  virtual std::string getTestDisplayName() = 0;
  virtual std::string getUniqueIdentifier() = 0;
  virtual ~Test() {}

  enum TestKind {
    TK_SimpleTest,
    TK_GoogleTest,
    TK_RustTest,
    TK_XCTest
  };
  TestKind getKind() const { return Kind; }
  Test(TestKind K) : Kind(K) {}
private:
  const TestKind Kind;
};

}
