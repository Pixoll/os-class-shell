#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "command.h"

#define MAX_COMMANDS 100
#define MAX_LENGTH 1024

typedef struct {
    int id;
    char command[MAX_LENGTH];
} FavoriteCommand;

FavoriteCommand favs[MAX_COMMANDS];
int fav_count = 0;
char fav_file[MAX_LENGTH] = "";

void agregar_favorito(const char *cmd) {
    for (int i = 0; i < fav_count; i++) {
        if (strcmp(favs[i].command, cmd) == 0) {
            return;
        }
    }
    if (fav_count < MAX_COMMANDS) {
        favs[fav_count].id = fav_count + 1;
        strncpy(favs[fav_count].command, cmd, MAX_LENGTH);
        fav_count++;
    }
}

void guardar_favoritos() {
    if (strlen(fav_file) == 0) {
        printf("No se ha creado el archivo de favoritos.\n");
        return;
    }

    FILE *file = fopen(fav_file, "w");
    if (!file) {
        perror("Error al abrir el archivo");
        return;
    }

    for (int i = 0; i < fav_count; i++) {
        fprintf(file, "%d %s\n", favs[i].id, favs[i].command);
    }
    fclose(file);
    printf("Favoritos guardados en %s\n", fav_file);
}

void cargar_favoritos() {
    if (strlen(fav_file) == 0) {
        printf("No se ha especificado un archivo de favoritos.\n");
        return;
    }

    FILE *file = fopen(fav_file, "r");
    if (!file) {
        perror("Error al abrir el archivo");
        return;
    }

    fav_count = 0;
    while (fscanf(file, "%d %[^\n]", &favs[fav_count].id, favs[fav_count].command) != EOF) {
        fav_count++;
    }

    fclose(file);
    printf("Favoritos cargados desde %s\n", fav_file);
}

void mostrar_favoritos() {
    if (fav_count == 0) {
        printf("No hay comandos en favoritos.\n");
        return;
    }
    for (int i = 0; i < fav_count; i++) {
        printf("%d: %s\n", favs[i].id, favs[i].command);
    }
}

void eliminar_favoritos(const char *nums) {
    char *token = strtok((char *)nums, ",");
    while (token) {
        int num = atoi(token);
        for (int i = 0; i < fav_count; i++) {
            if (favs[i].id == num) {
                for (int j = i; j < fav_count - 1; j++) {
                    favs[j] = favs[j + 1];
                }
                fav_count--;
                break;
            }
        }
        token = strtok(NULL, ",");
    }
}

void buscar_favoritos(const char *cmd) {
    int encontrado = 0;
    for (int i = 0; i < fav_count; i++) {
        if (strstr(favs[i].command, cmd)) {
            printf("%d: %s\n", favs[i].id, favs[i].command);
            encontrado = 1;
        }
    }
    if (!encontrado) {
        printf("No se encontraron comandos que contengan '%s'.\n", cmd);
    }
}

void ejecutar_favorito(int num) {
    for (int i = 0; i < fav_count; i++) {
        if (favs[i].id == num) {
            printf("Ejecutando: %s\n", favs[i].command);
            system(favs[i].command);
            return;
        }
    }
    printf("No se encontró el comando con el número %d.\n", num);
}

int exec_favs(Command command) {
    //Lógica de manejo de favoritos
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
}