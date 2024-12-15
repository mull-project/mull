// RUN: true
#include "math.h"

int main(int argc, char **argv) {
  add(0, 0); // making sure it's mutated
  return !(sub(12, 10) == 2);
}
