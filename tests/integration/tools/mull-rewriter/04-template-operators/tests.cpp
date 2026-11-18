// The mutated code
int signed_shift(int n);
unsigned unsigned_shift(unsigned n);
unsigned top_bit(unsigned n);
long length(int n);
int mixed(int n);

// The same code, not mutated: the expected results
namespace reference {
#include "code.cpp"
}

int main() {
  for (int n = 1; n <= 3; n++) {
    bool same = signed_shift(n) == reference::signed_shift(n) &&
                unsigned_shift(n) == reference::unsigned_shift(n) &&
                top_bit(n) == reference::top_bit(n) && length(n) == reference::length(n) &&
                mixed(n) == reference::mixed(n);
    if (!same) {
      return 1;
    }
  }
  return 0;
}
