#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "prompt.h"

int exec_set_recordatorio(const int argc, const char **argv) {
    if (argc == 0 || argv[1] == NULL || strcmp(argv[1], "recordatorio") != 0) {
        fprintf(stderr, "set: command not found\n");
        return 1;
    }

    if (argc != 4) {
        fprintf(stderr, "Usage: set recordatorio <seconds> \"<message>\"\n");
        return 1;
    }

    char *end;
    const int seconds = strtol(argv[2], &end, 10);
    if (end[0] != 0) {
        fprintf(stderr, "set: invalid seconds\n");
        return 1;
    }

    const pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        sleep(seconds);
        printf("\nset recordatorio:\nReminder: %s\nSet %d seconds ago\n", argv[3], seconds);
        prompt();
        exit(0);
    }

    printf("Reminder set\n");
    return 0;
}
