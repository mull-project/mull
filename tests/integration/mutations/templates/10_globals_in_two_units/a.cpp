struct S {};
S operator+(S, S);

#include "globals.h"

int b();

int main() {
  return Counter<int>::start != 2 || scaled<int> != 3 || shared != 4 || b() != 9;
}
