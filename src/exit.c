#include "exit.h"

#include <stdio.h>
#include <stdlib.h>

int exec_exit(int argc, const char **argv) {
    printf("Bye!\n");
    exit(0);
}
