#include "command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Command read_command() {
    int command_buffer_size = 64;
    char *command = malloc(command_buffer_size);
    int c, command_length = 0;
    bool piped = false;

    while ((c = getchar()) != '\n' && c != '\r') {
        if (c == EOF) {
            printf("\nBye!\n");
            exit(EOF);
        }

        if (c == '|')
            piped = true;

        if (command_length >= command_buffer_size) {
            command_buffer_size *= 2;

            char *reallocated = realloc(command, command_buffer_size);
            if (reallocated == NULL) {
                free(command);
                printf("\nOut of memory!\n");
                exit(EXIT_FAILURE);
            }

            command = reallocated;
        }

        command[command_length++] = c;
    }

    if (command_length == 0) {
        free(command);
        return EMPTY_COMMAND;
    }

    command[command_length] = '\0';

    const ProcessArgs args = parse_args(command);

    return (Command){command, piped, args.argc, args.argv};
}

void add_arg(char *arg, int *arg_len, char ***argv, int *argc, int *args_buffer_size) {
    if (*arg_len == 0)
        return;

    if (*argc >= *args_buffer_size) {
        *args_buffer_size += 64;

        char **reallocated = realloc(*argv, *args_buffer_size * sizeof(char *));
        if (reallocated == NULL) {
            free(*argv);
            printf("\nOut of memory!\n");
            exit(EXIT_FAILURE);
        }

        *argv = reallocated;
    }

    arg[*arg_len] = 0;
    (*argv)[*argc] = strdup(arg);
    *argc += 1;

    for (int i = 0; i < *arg_len; i++)
        arg[i] = 0;
    *arg_len = 0;
}

ProcessArgs parse_args(const char *command) {
    const int command_len = strlen(command);

    int args_buffer_size = 64;
    int argc = 0;
    char **argv = malloc(args_buffer_size * sizeof(char *));

    int arg_len = 0;
    char arg[command_len];
    arg[0] = 0;

    for (int i = 0; i < command_len; i++) {
        const char c = command[i];

        if (c == '"') {
            add_arg(arg, &arg_len, &argv, &argc, &args_buffer_size);

            const int j = i++;
            bool escaped = false;

            while (i < command_len && (!escaped ? command[i] != '"' : true)) {
                if (escaped)
                    escaped = false;

                if (command[i] == '\\')
                    escaped = true;

                arg[arg_len++] = command[i++];
            }

            if (command[i] != '"')
                fprintf(stderr, "Unmatched quote:\n%s\n%*s^ here\n", command, j, "");

            add_arg(arg, &arg_len, &argv, &argc, &args_buffer_size);
            continue;
        }

        if (c != ' ' && c != '\t') {
            arg[arg_len++] = c;

            if (i < command_len - 1)
                continue;
        }

        add_arg(arg, &arg_len, &argv, &argc, &args_buffer_size);
    }

    argv[argc] = NULL;

    return (ProcessArgs){argc, argv};
}

bool is_command_empty(const Command command) {
    return command.input == NULL || command.argc == 0 || command.argv == NULL;
}

void free_command(const Command command) {
    free(command.input);
    for (int i = 0; i < command.argc; i++)
        free(command.argv[i]);
    free(command.argv);
}
