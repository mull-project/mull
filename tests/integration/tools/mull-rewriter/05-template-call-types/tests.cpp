// The mutated code
unsigned wide_width();
unsigned narrow_width();

// The same code, not mutated: the expected results
namespace reference {
#include "code.cpp"
}

int main() {
  bool same =
      wide_width() == reference::wide_width() && narrow_width() == reference::narrow_width();
  return same ? 0 : 1;
}
