// The mutated code
extern int consumed;
int arithmetic(int a, int b);
int bitwise(int a, int b);
int comparison(int a, int b);
int increments(int a);
bool logical(int a, int b);
int negation(int a);
int calls(int a);
int constants(int a);

// The same code, not mutated: the expected results
namespace reference {
#include "code.cpp"
}

int main() {
  for (int a = -3; a <= 3; a++) {
    for (int b = -3; b <= 3; b++) {
      bool same = arithmetic(a, b + 4) == reference::arithmetic(a, b + 4) &&
                  bitwise(a, b) == reference::bitwise(a, b) &&
                  comparison(a, b) == reference::comparison(a, b) &&
                  increments(a) == reference::increments(a) &&
                  logical(a, b) == reference::logical(a, b) &&
                  negation(a) == reference::negation(a) && calls(a) == reference::calls(a) &&
                  consumed == reference::consumed && constants(a) == reference::constants(a);
      if (!same) {
        return 1;
      }
    }
  }
  return 0;
}
