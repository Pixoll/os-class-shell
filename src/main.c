// ReSharper disable CppDFAEndlessLoop
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_COMMAND 1024

void execute_command(char **args) {
    const pid_t pid = fork();

    // error
    if (pid < 0) {
        perror("fork");
        return;
    }

    // child
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("execvp");
        }
        exit(EXIT_FAILURE);
    }

    // parent
    int state;
    do {
        waitpid(pid, &state, WUNTRACED);
    } while (!WIFEXITED(state) && !WIFSIGNALED(state));
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
                perror("execvp");
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
    char command[MAX_COMMAND];
    int c, i = 0;
    *piped = *args_count = 0;

    while ((c = getchar()) != '\n' && c != EOF) {
        command[i++] = c;
        if (c == '|')
            *piped = 1;
    }

    if (i == 0)
        return NULL;

    command[i] = '\0';

    int buffer_size = 64;
    i = 0;

    char **args = malloc(buffer_size * sizeof(char *));
    char *arg = strtok(command, " \t\r\n");

    while (arg != NULL) {
        args[i++] = arg;

        if (i >= buffer_size) {
            buffer_size += 64;
            args = realloc(args, buffer_size * sizeof(char *));
        }

        arg = strtok(NULL, " \t\r\n");
    }

    *args_count = i;
    args[i] = NULL;

    return args;
}

int main() {
    do {
        printf("> ");
        int arg_count, piped;
        char **args = read_command(&arg_count, &piped);

        if (arg_count == 0 || args == NULL)
            continue;

        if (piped)
            execute_pipes(args);
        else
            execute_command(args);

        free(args);
    } while (1);

    return 0;
}
