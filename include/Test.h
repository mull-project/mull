#pragma once

#include <string>

namespace Mutang {

class Test {
public:
  virtual std::string getTestName() = 0;
  virtual ~Test() {}

  enum TestKind {
    TK_SimpleTest,
    TK_GoogleTest
  };
  TestKind getKind() const { return Kind; }
  Test(TestKind K) : Kind(K) {}
private:
  const TestKind Kind;
};

}
