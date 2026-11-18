struct S {};
void operator<<(S, int);

#include "shift.h"

int main() {
  return shl1(1u) != 2;
}
