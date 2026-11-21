// The mutated code
long long unwrap_narrow(int n);
long long unwrap_wide(int n);

// The same code, not mutated: the expected results
namespace reference {
#include "narrow.cpp"
#include "wide.cpp"
} // namespace reference

int main() {
  for (int n = 1; n <= 3; n++) {
    bool same = unwrap_narrow(n) == reference::unwrap_narrow(n) &&
                unwrap_wide(n) == reference::unwrap_wide(n);
    if (!same) {
      return 1;
    }
  }
  return 0;
}
