#include <assert.h>

// Returns 1 if value is in the closed interval [min, max], 0 otherwise
int in_range(int value, int min, int max) {
  return value >= min && value < max;
}

int main(void) {
  assert(in_range(5,  1, 10) == 1);  // mid-range: both conditions true
  assert(in_range(0,  1, 10) == 0);  // below min: first condition false
  assert(in_range(15, 1, 10) == 0);  // above max: second condition false
  return 0;
}
