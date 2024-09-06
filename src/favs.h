#ifndef FAVS_H
#define FAVS_H

#define MAX_COMMANDS 100
#define MAX_LENGTH 1024

int fav_count;
char fav_file[MAX_LENGTH];

void agregar_favorito(const char *cmd);
void guardar_favoritos();
void cargar_favoritos();
void mostrar_favoritos();
void eliminar_favoritos(const char *nums);
void buscar_favoritos(const char *cmd);
void ejecutar_favorito(int num);

#endif
