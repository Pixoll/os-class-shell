// ReSharper disable CppDFAEndlessLoop
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#include "command.h"
#include "execute.h"
#include "recordatorio.h"

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif

Command last_command = {NULL, false, 0, NULL};

void handle_sigint(int code);
char *replace_str(char *str, const char *orig, char *rep, int start);

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
        if(strncmp(command.command, "set recordatorio", 16) == 0) {
            int time;
            char msg[256];
            if(sscanf(command.command, "set recordatorio %d \"%[^\"]\"", &time, msg) == 2) {
                recordatorio(time,msg);
            }else {
                printf("Error: Comando incorrecto. Uso: set recordatorio <segundos> \"<mensaje>\"\n");
            }

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
