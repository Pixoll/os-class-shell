#include "command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Command read_command() {
    int command_buffer_size = 64;
    char *command = malloc(command_buffer_size);
    int c, command_length = 0;
    bool piped = false;

    while ((c = getchar()) != '\n') {
        if (c == EOF) {
            printf("\nBye!\n");
            exit(EOF);
        }

        if (c == '|')
            piped = true;

        if (command_length >= command_buffer_size) {
            command_buffer_size *= 2;
            command = realloc(command, command_buffer_size);
        }

        command[command_length++] = c;
    }

    if (command_length == 0) {
        free(command);
        return (Command){NULL, false, 0, NULL};
    }

    command[command_length] = '\0';

    const ProcessArgs args = parse_args(command);

    return (Command){command, piped, args.argc, args.argv};
}

ProcessArgs parse_args(const char *command) {
    char *command_dup = strdup(command);
    int args_buffer_size = 64;
    int argc = 0;

    char **argv = malloc(args_buffer_size * sizeof(char *));
    const char *arg = strtok(command_dup, " \t\r\n");

    while (arg != NULL) {
        argv[argc++] = strdup(arg);

        if (argc >= args_buffer_size) {
            args_buffer_size += 64;
            argv = realloc(argv, args_buffer_size * sizeof(char *));
        }

        arg = strtok(NULL, " \t\r\n");
    }

    argv[argc] = NULL;
    free(command_dup);

    return (ProcessArgs){argc, argv};
}

bool is_command_empty(const Command command) {
    return command.command == NULL || command.argc == 0 || command.argv == NULL;
}

void free_command(const Command command) {
    free(command.command);
    for (int i = 0; i < command.argc; i++)
        free(command.argv[i]);
    free(command.argv);
}
