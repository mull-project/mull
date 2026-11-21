// The mutated code
int flag();
int number();

// The same code, not mutated: the expected results
namespace reference {
#include "code.cpp"
}

int main() {
  // Tells 42 from 1, the values of the call mutant in the IR and in the rewritten program
  bool same = flag() == reference::flag() && (number() < 10) == (reference::number() < 10);
  return same ? 0 : 1;
}
