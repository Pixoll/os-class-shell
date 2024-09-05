// ReSharper disable CppDFAEndlessLoop
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "command.h"
#include "execute.h"
#include "prompt.h"
#include "recordatorio.h"

Command last_command = {NULL, false, 0, NULL};

void handle_sigint(int code);

int main() {
    signal(SIGINT, handle_sigint);
    printf("+-------+\n| Shell |\n+-------+\n\n");

    do {
        prompt();
        Command command = read_command();

        if (is_command_empty(command))
            continue;

        if (!is_command_empty(last_command) && strcmp(command.command, "!!") == 0) {
            free_command(command);
            printf("%s\n", last_command.command);
            command = last_command;
        }
        // if(strncmp(command.command, "set recordatorio", 16) == 0) {
        //     int time;
        //     char msg[256];
        //     if(sscanf(command.command, "set recordatorio %d \"%[^\"]\"", &time, msg) == 2) {
        //         exec_set_recordatorio(time,msg);
        //     }else {
        //         printf("Error: Comando incorrecto. Uso: set recordatorio <segundos> \"<mensaje>\"\n");
        //     }
        //
        // }

        if (command.piped)
            execute_pipes(command.argv);
        else
            execute_command(command.argc, command.argv);

        if (!is_command_empty(last_command))
            free_command(last_command);

        last_command = command;
    } while (1);
}

void handle_sigint(const int code) {
    printf("\nBye!\n");
    exit(code);
}
