#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "prompt.h"

int exec_set_recordatorio(const int argc, const char **argv) {
    if (argc == 0 || argv[1] == NULL || (strcmp(argv[1], "recordatorio") != 0 && strcmp(argv[1], "reminder") != 0)) {
        fprintf(stderr, "set: command not found\n");
        return 1;
    }

    if (argc != 4) {
        fprintf(stderr, "Usage: set recordatorio|reminder <seconds> \"<message>\"\n");
        return 1;
    }

    char *end;
    const int seconds = strtol(argv[2], &end, 10);
    if (end[0] != 0) {
        fprintf(stderr, "set reminder: invalid seconds\n");
        return 1;
    }

    const pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        sleep(seconds);
        printf("\nset reminder:\n\nReminder: \x1b[1;36m%s\x1b[0m\n\nSet: \x1b[1;33m%d seconds ago\x1b[0m\n", argv[3], seconds);
        prompt();
        exit(0);
    }

    printf("Reminder set\n");
    return 0;
}
