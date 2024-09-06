#include "execute.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "cd.h"
#include "exit.h"
#include "favs.h"
#include "recordatorio.h"

typedef struct BuiltinCommand {
    char *name;
    int (*run)(int argc, const char **argv);
} BuiltinCommand;

static const BuiltinCommand builtin_commands[] = {
    {"cd", exec_cd},
    {"exit", exec_exit},
    {"set", exec_set_recordatorio},
    {"favs", exec_favs}
};

static const int builtins_amount = sizeof(builtin_commands) / sizeof(BuiltinCommand);

void execute_command(const int argc, char **argv) {
    for (int i = 0; i < builtins_amount; i++) {
        if (strcmp(builtin_commands[i].name, argv[0]) == 0) {
            builtin_commands[i].run(argc, (const char **)argv);
            return;
        }
    }

    const pid_t pid = fork();

    // error
    if (pid < 0) {
        perror("fork");
        return;
    }

    // child
    if (pid == 0) {
        if (execvp(argv[0], argv) == -1) {
            if (errno == ENOENT)
                fprintf(stderr, "%s: command not found\n", argv[0]);
            else
                perror("execvp");
        }

        exit(EXIT_FAILURE);
    }

    // parent
    int status;
    do {
        if (waitpid(pid, &status, WUNTRACED | WCONTINUED) == -1) {
            perror("waitpid");
            break;
        }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
}

void execute_pipes(char **args) {
    int pipe_count = 0;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_count++;
        }
    }

    char *commands[pipe_count + 1][64];
    int command_index = 0, arg_index = 0;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            commands[command_index][arg_index] = NULL;
            command_index++;
            arg_index = 0;
        } else {
            commands[command_index][arg_index++] = args[i];
        }
    }
    commands[command_index][arg_index] = NULL;

    int fd_in = 0;

    for (int i = 0; i <= pipe_count; i++) {
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            dup2(fd_in, STDIN_FILENO);

            if (i < pipe_count) {
                dup2(pipe_fd[1], STDOUT_FILENO);
            }

            close(pipe_fd[0]);
            execute_command(arg_index, commands[i]);
            exit(EXIT_FAILURE);
        }

        close(pipe_fd[1]);
        fd_in = pipe_fd[0];
        wait(NULL);
    }
}
