extern "C" {
int position = 0;
int advance() {
  return ++position;
}
void ignore(int) {}
bool present(const int *value) {
  return value != nullptr;
}
}

// The mutated code
int ignored();
int filled();
int counted();
int unused(int n);
int taken();

// The same code, not mutated: the expected results
namespace reference {
#include "code.cpp"
}

int main() {
  bool same = ignored() == reference::ignored() && filled() == reference::filled() &&
              counted() == reference::counted() && unused(3) == reference::unused(3) &&
              taken() == reference::taken();
  return same ? 0 : 1;
}
