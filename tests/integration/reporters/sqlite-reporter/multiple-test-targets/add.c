// RUN: true
#include "math.h"

int main(int argc, char **argv) {
  sub(0, 0); // making sure it's mutated
  return !(add(10, 12) == 22);
}
