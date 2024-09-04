// ReSharper disable CppDFAEndlessLoop
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <bits/local_lim.h>
#include <linux/limits.h>
#include <sys/wait.h>

#include "cd.h"
#include "exit.h"

#define CUSTOM_COMMANDS 2

typedef struct Command {
    char *command;
    bool piped;
    int argc;
    char **argv;
} Command;

typedef struct CustomCommand {
    char *name;
    int (*command)(int argc, const char **argv);
} CustomCommand;

const CustomCommand custom_commands[CUSTOM_COMMANDS] = {
    {"cd", exec_cd},
    {"exit", exec_exit},
};

Command last_command = {NULL, false, 0, NULL};

void handle_sigint(int code);
char *replace_str(char *str, const char *orig, char *rep, int start);
Command read_command();
bool is_command_empty(Command command);
void free_command(Command command);
void execute_command(int argc, char **argv);
void execute_pipes(char **args);

int main() {
    signal(SIGINT, handle_sigint);

    printf("+-------+\n| Shell |\n+-------+\n\n");

    const char *home_path = getenv("HOME");

    char hostname[HOST_NAME_MAX + 1];
    gethostname(hostname, sizeof(hostname));

    do {
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        replace_str(cwd, home_path, "~", 0);
        printf("\x1b[1;32m%s@%s\x1b[0m:\x1b[1;34m%s\x1b[0m$ ", getenv("USER"), hostname, cwd);

        Command command = read_command();

        if (is_command_empty(command))
            continue;

        if (!is_command_empty(last_command) && strcmp(command.command, "!!") == 0) {
            free_command(command);
            printf("%s\n", last_command.command);
            command = last_command;
        }

        if (command.piped)
            execute_pipes(command.argv);
        else
            execute_command(command.argc, command.argv);

        if (!is_command_empty(last_command))
            free_command(last_command);

        last_command = command;
    } while (1);
}

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

Command read_command() {
    int command_buffer_size = 64;
    char *command = malloc(command_buffer_size);
    int c, command_length = 0;
    bool piped = false;

    while ((c = getchar()) != '\n') {
        if (c == EOF)
            handle_sigint(EOF);

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

    return (Command){command, piped, argc, argv};
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

char *replace_str(char *str, const char *orig, char *rep, const int start) {
    static char temp[4096];
    static char buffer[4096];
    char *p;

    strcpy(temp, str + start);

    if (!((p = strstr(temp, orig))))  // Is 'orig' even in 'temp'?
        return temp;

    strncpy(buffer, temp, p - temp); // Copy characters from 'temp' start to 'orig' str
    buffer[p - temp] = '\0';

    sprintf(buffer + (p - temp), "%s%s", rep, p + strlen(orig));
    sprintf(str + start, "%s", buffer);

    return str;
}

void handle_sigint(const int code) {
    printf("\nBye!\n");
    exit(code);
}
