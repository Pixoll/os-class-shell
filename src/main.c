#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_COMANDO 1024

void ejecutar_comando(char **args) {
    pid_t pid, wpid;
    int estado;
    pid = fork();
    if (pid == 0) {
        if(execvp(args[0], args) == -1) {
            perror("execvp");
        }
        exit(EXIT_FAILURE);
    } else if(pid < 0) {
        perror("fork");
    } else {
        do {
            wpid = waitpid(pid, &estado, WUNTRACED);
        } while (!WIFEXITED(estado) && !WIFSIGNALED(estado));
    }
}

void ejecutar_pipies(char **args) {
    int pipefd[2];
    pid_t pid;
    int fd_in = 0;

    while (*args != NULL) {
        pipe(pipefd);
        pid = fork();

        if (pid == 0) {
            dup2(fd_in, 0);
            if (*(args + 1) != NULL) {
                dup2(pipefd[1], 1);
            }
            close(pipefd[0]);
            if (execvp(*args, args) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else {
            wait(NULL);
            close(pipefd[1]);
            fd_in = pipefd[0];
            args++;
        }
    }
}

char **analizar_input(char* command) {
    int tamaño_buffer = 64;
    int i = 0;
    char *arg;
    char **args = malloc(tamaño_buffer * sizeof(char*));

    arg = strtok(command, " \t\r\n");
    while (arg != NULL) {
        args[i] = arg;
        i++;
        if (i >= tamaño_buffer) {
            tamaño_buffer += 64;
            args = realloc(args, tamaño_buffer * sizeof(char*));
        }
        arg = strtok(NULL, " \t\r\n");
    }
    args[i] = NULL;
    return args;
}

char *leer_comando(void) {
    char *comando = malloc(MAX_COMANDO * sizeof(char));
    int c;
    int i = 0;

    while ((c = getchar()) != '\n' && c != EOF) {
        comando[i] = c;
        i++;
    }
    comando[i] = '\0';  // Null-terminate the string
    return comando;
}

int main(const int argc, const char **argv) {
    char *input;
    char **args;

    do {
        printf("> ");
        input = leer_comando();
        args = analizar_input(input);

        if (args[0] != NULL) {
            if (strchr(input, '|') != NULL) {
                ejecutar_pipies(args);
            } else {
                ejecutar_comando(args);
            }
        }

        free(input);
        free(args);

    } while (1);

    return 0;
}
