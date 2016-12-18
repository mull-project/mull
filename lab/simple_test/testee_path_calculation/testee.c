#include "testee.h"

#include <stdio.h>

int testee4() {
    return 1;
}

int testee3() {
    return testee4();
}

int testee2() {
    return testee3();
}

int testee1() {
    return testee2();
}

