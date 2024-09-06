// ReSharper disable CppDFAEndlessLoop
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "command.h"
#include "execute.h"
#include "prompt.h"
#include "favs.h"
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

        if (strncmp(command.command, "favs", 4) == 0) {
            // Lógica de manejo de favoritos
            if (strncmp(command.command, "favs crear ", 11) == 0) {
                strncpy(fav_file, command.command + 11, MAX_LENGTH);
                FILE *file = fopen(fav_file, "w");
                if (file) {
                    fclose(file);
                    printf("Archivo de favoritos creado en: %s\n", fav_file);
                } else {
                    perror("Error al crear el archivo");
                }
            } else if (strcmp(command.command, "favs mostrar") == 0) {
                mostrar_favoritos();
            } else if (strncmp(command.command, "favs eliminar ", 14) == 0) {
                eliminar_favoritos(command.command + 14);
            } else if (strncmp(command.command, "favs buscar ", 12) == 0) {
                buscar_favoritos(command.command + 12);
            } else if (strcmp(command.command, "favs borrar") == 0) {
                fav_count = 0;
                printf("Todos los favoritos han sido borrados.\n");
            } else if (strncmp(command.command, "favs ", 5) == 0 && strstr(command.command, " ejecutar") != NULL) {
                int num = atoi(command.command + 5);
                ejecutar_favorito(num);
            } else if (strcmp(command.command, "favs cargar") == 0) {
                cargar_favoritos();
            } else if (strcmp(command.command, "favs guardar") == 0) {
                guardar_favoritos();
            } else {
                agregar_favorito(command.command);
                system(command.command);
            }

            if (command.piped)
                execute_pipes(command.argv);
            else
                execute_command(command.argc, command.argv);

            if (!is_command_empty(last_command))
                free_command(last_command);

            last_command = command;
        }
        while (1);
    }



    void handle_sigint(const int code) {
        printf("\nBye!\n");
        exit(code);
    }
