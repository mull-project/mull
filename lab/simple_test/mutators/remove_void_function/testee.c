#include "testee.h"

#include <stdio.h>

static int global_state = 0;

void void_function() {
    printf("Void function is called\n");
    global_state = 1;
}

int testee() {
    printf("Before void function is called\n");
    void_function();
    printf("After void function is called\n");
    if (global_state == 0) {
        return 0;
    }
    return 1;
}

