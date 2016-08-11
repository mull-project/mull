#pragma once

namespace Mutang {

class Test {
public:
  enum TestKind {
    TK_SimpleTest
  };
  TestKind getKind() const { return Kind; }
  Test(TestKind K) : Kind(K) {}
private:
  const TestKind Kind;
};

}
