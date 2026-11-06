// The mutated code
enum class color { red, green };
int resized(int n);
int assigned_digits(int n);
int assigned_exp(int n);
int limit();
char letter();
color picked(int n);
int unwrapped(int n);
int sum(int n);
long length(int n);

// The same code, not mutated: the expected results
namespace reference {
#include "code.cpp"
}

int main() {
  for (int n = 1; n <= 3; n++) {
    bool same = resized(n) == reference::resized(n) &&
                assigned_digits(n) == reference::assigned_digits(n) &&
                assigned_exp(n) == reference::assigned_exp(n) && limit() == reference::limit() &&
                letter() == reference::letter() && int(picked(n)) == int(reference::picked(n)) &&
                unwrapped(n) == reference::unwrapped(n) && sum(n) == reference::sum(n) &&
                length(n) == reference::length(n);
    if (!same) {
      return 1;
    }
  }
  return 0;
}
