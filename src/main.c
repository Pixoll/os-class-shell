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

char **parse_args(char *command) {
    int buffer_size = 64;
    int i = 0;

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

    args[i] = NULL;
    return args;
}

char *read_command() {
    char *command = malloc(MAX_COMMAND * sizeof(char));
    int c;
    int i = 0;

    while ((c = getchar()) != '\n' && c != EOF)
        command[i++] = c;

    command[i] = '\0';  // Null-terminate the string
    return command;
}

int main() {
    do {
        printf("> ");
        char *input = read_command();
        char **args = parse_args(input);

        if (args[0] != NULL) {
            if (strchr(input, '|') != NULL)
                execute_pipes(args);
            else
                execute_command(args);
        }

        free(input);
        free(args);
    } while (1);

    return 0;
}
