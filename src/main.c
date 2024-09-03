// ReSharper disable CppDFAEndlessLoop
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/wait.h>

#include "cd.h"
#include "exit.h"

#define CUSTOM_COMMANDS 2

typedef struct Command {
    char *name;
    int (*command)(int argc, const char **argv);
} Command;

const Command custom_commands[CUSTOM_COMMANDS] = {
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

char **read_command(int *args_count, int *piped) {
    int command_buffer_size = 64;
    char *command = malloc(command_buffer_size);
    int c, i = 0;
    *piped = *args_count = 0;

    while ((c = getchar()) != '\n' && c != EOF) {
        if (i >= command_buffer_size) {
            command_buffer_size *= 2;
            command = realloc(command, command_buffer_size);
        }

        command[i++] = c;
        if (c == '|')
            *piped = 1;
    }

    if (i == 0) {
        free(command);
        return NULL;
    }

    command[i] = '\0';

    int args_buffer_size = 64;
    i = 0;

    char **args = malloc(args_buffer_size * sizeof(char *));
    const char *arg = strtok(command, " \t\r\n");

    while (arg != NULL) {
        args[i++] = strdup(arg);

        if (i >= args_buffer_size) {
            args_buffer_size += 64;
            args = realloc(args, args_buffer_size * sizeof(char *));
        }

        arg = strtok(NULL, " \t\r\n");
    }

    *args_count = i;
    args[i] = NULL;
    free(command);

    return args;
}

int main() {
    do {
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        printf("%s$ ", cwd);

        int arg_count, piped;
        char **args = read_command(&arg_count, &piped);

        if (arg_count == 0 || args == NULL)
            continue;

        if (piped)
            execute_pipes(args);
        else
            execute_command(arg_count, args);

        for (int i = 0; i < arg_count; i++)
            free(args[i]);
        free(args);
    } while (1);

    return 0;
}
