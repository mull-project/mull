#include "testee.h"

#include <stdio.h>

int testee4(int x) {
    /// this trivial mutation is needed to get 
    /// non-empty array of mutation results.
    int result = 20 + x;

    printf("testee4 returns result: %d\n", result);
    return result;
}

int testee3() {
    return testee4(1);
}

int testee2() {
    return testee3();
}

int testee1() {
    return testee2();
}

