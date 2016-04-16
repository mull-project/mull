#include "Laboratory.h"
#include <stdio.h>

using namespace Mutang;

const char *Laboratory::name() {
  return "some lab name";
}

void Laboratory::run() {
  printf("%s\n", name());
}

