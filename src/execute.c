#include "execute.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "cd.h"
#include "exit.h"

#define CUSTOM_COMMANDS 2

typedef struct CustomCommand {
    char *name;
    int (*command)(int argc, const char **argv);
} CustomCommand;

const CustomCommand custom_commands[CUSTOM_COMMANDS] = {
    {"cd", exec_cd},
    {"exit", exec_exit},
};

void execute_command(const int argc, char **argv) {
    for (int i = 0; i < CUSTOM_COMMANDS; i++) {
        if (strcmp(custom_commands[i].name, argv[0]) == 0) {
            custom_commands[i].command(argc, argv);
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
    int fd_in = 0;

    while (*args != NULL) {
        int pipe_fd[2];
        pipe(pipe_fd);
        const pid_t pid = fork();

        // error
        if (pid < 0) {
            perror("fork");
            return;
        }

        // child
        if (pid == 0) {
            dup2(fd_in, 0);
            if (*(args + 1) != NULL) {
                dup2(pipe_fd[1], 1);
            }

            close(pipe_fd[0]);

            if (execvp(*args, args) == -1) {
                if (execvp(args[0], args) == -1) {
                    if (errno == ENOENT)
                        fprintf(stderr, "%s: command not found\n", args[0]);
                    else
                        perror("execvp");
                }

                exit(EXIT_FAILURE);
            }

            continue;
        }

        // parent
        wait(NULL);
        close(pipe_fd[1]);
        fd_in = pipe_fd[0];
        args++;
    }
}
