#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>

typedef struct Command {
    char *command;
    bool piped;
    int argc;
    char **argv;
} Command;

typedef struct ProcessArgs {
    int argc;
    char **argv;
} ProcessArgs;

Command read_command();
ProcessArgs parse_args(const char *command);
bool is_command_empty(Command command);
void free_command(Command command);

#endif
