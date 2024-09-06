#include "favs.h"

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#include "command.h"

// forward declaration, #include causes circular dependency
void execute_command(int argc, char **argv);

typedef struct Subcommand {
    const char *name;
    int (*run)(const char **args);
} Subcommand;

typedef struct FavCommands {
    int allocated;
    int amount;
    char **list;
} FavCommands;

int create_fav_commands(const char **args);
int show_fav_commands(const char **);
int delete_fav_commands(const char **args);
int search_fav_commands(const char **args);
int clear_fav_commands(const char **);
int run_fav_commands(const char **args);
int load_fav_commands(const char **);
int save_fav_commands(const char **);

static const Subcommand subcommands[] = {
    {"crear", create_fav_commands},
    {"create", create_fav_commands},
    {"mostrar", show_fav_commands},
    {"show", show_fav_commands},
    {"eliminar", delete_fav_commands},
    {"delete", delete_fav_commands},
    {"buscar", search_fav_commands},
    {"search", search_fav_commands},
    {"borrar", clear_fav_commands},
    {"clear", clear_fav_commands},
    {"ejecutar", run_fav_commands},
    {"run", run_fav_commands},
    {"cargar", load_fav_commands},
    {"load", load_fav_commands},
    {"guardar", save_fav_commands},
    {"save", save_fav_commands},
};

static const int subcommands_amount = sizeof(subcommands) / sizeof(Subcommand);

static const char *config_file_name = "myshell_favs.conf";
static char config_path[PATH_MAX];
static char fav_commands_path[PATH_MAX];

static FavCommands fav_commands = {0, 0, NULL};

static const char *usage_string = "Usage:\n\n"
        "favs crear|create <path>          --  create new favorite commands file\n"
        "favs mostrar|show                 --  show favorite commands\n"
        "favs eliminar|delete <number>...  --  delete favorite commands\n"
        "favs buscar|search <cmd>          --  search for matches in your favorite commands\n"
        "favs borrar|clear                 --  clear all your favorite commands\n"
        "favs ejecutar|run <number>        --  run a favorite command\n"
        "favs <number> ejecutar|run        --  run a favorite command\n"
        "favs cargar|load                  --  load your favorite commands file and show its contents\n"
        "favs guardar|save                 --  save favorite commands\n";

bool is_digits_only(const char *s) {
    if (s == NULL || s[0] == 0)
        return false;
    while (*s)
        if (isdigit(*s++) == 0)
            return false;
    return true;
}

void load_config_file();

int exec_favs(const int argc, const char **argv) {
    if (config_path[0] == 0) {
        const char *home_dir = getenv("HOME");
        const int config_path_length = strlen(home_dir) + strlen(config_file_name) + 2;
        strcpy(config_path, home_dir);
        strcat(config_path, "/");
        strcat(config_path, config_file_name);
        config_path[config_path_length - 1] = 0;
    }

    load_config_file();

    if (argc < 2) {
        fprintf(stderr, "%s", usage_string);
        return 1;
    }

    const char *subcommand = argv[1];

    for (int i = 0; i < subcommands_amount; i++)
        if (strcmp(subcommand, subcommands[i].name) == 0)
            return subcommands[i].run(argv + 2);

    if (argc == 3 && is_digits_only(argv[1]) && (strcmp(argv[2], "ejecutar") || strcmp(argv[2], "run")))
        return run_fav_commands(argv + 1);

    fprintf(stderr, "%s", usage_string);
    return 1;
}

void add_to_favs(const char *command) {
    load_config_file();

    if (strncmp(command, "favs", 4) == 0 || fav_commands.allocated == 0)
        return;

    for (int i = 0; i < fav_commands.amount; i++)
        if (strcmp(command, fav_commands.list[i]) == 0)
            return;

    if (fav_commands.amount + 1 > fav_commands.allocated) {
        fav_commands.allocated *= 2;

        char **reallocated = realloc(fav_commands.list, fav_commands.allocated * sizeof(char *));
        if (reallocated == NULL) {
            free(fav_commands.list);
            printf("\nOut of memory!\n");
            exit(1);
        }

        fav_commands.list = reallocated;
    }

    fav_commands.list[fav_commands.amount++] = strdup(command);
}

void load_config_file() {
    if (fav_commands_path[0] != 0 || access(config_path, F_OK) != 0)
        return;

    FILE *config_file = fopen(config_path, "r");
    if (config_file == NULL) {
        fprintf(stderr, "Failed to open config file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    fgets(fav_commands_path, PATH_MAX, config_file);
    fclose(config_file);
}

void free_fav_commands() {
    for (int i = 0; i < fav_commands.amount; i++)
        free(fav_commands.list[i]);
    free(fav_commands.list);
    fav_commands.amount = 0;
    fav_commands.allocated = 1;

    fav_commands.list = malloc(sizeof(char *));
    fav_commands.list[0] = NULL;
}

int create_fav_commands(const char **args) {
    if (args[0] == NULL) {
        fprintf(stderr, "Usage: favs crear|create <path>\n");
        return 1;
    }

    if (fav_commands_path[0] != 0) {
        fprintf(stderr, "Favorite commands file already exists at %s\n", fav_commands_path);
        return 1;
    }

    if (access(args[0], F_OK) == 0) {
        fprintf(stderr, "That file already exists.\n");
        return 1;
    }

    if (access(config_path, F_OK) == 0) {
        FILE *config_file = fopen(config_path, "r");
        if (config_file == NULL) {
            fprintf(stderr, "Failed to open config file: %s\n", strerror(errno));
            return 1;
        }

        char temp_path[PATH_MAX];
        fgets(temp_path, PATH_MAX, config_file);

        fprintf(stderr, "Favorite commands file already exists at %s\n", temp_path);
        return 1;
    }

    realpath(args[0], fav_commands_path);

    FILE *config_file = fopen(config_path, "w");
    if (config_file == NULL) {
        fprintf(stderr, "Failed to create config file: %s\n", strerror(errno));
        return 1;
    }

    fprintf(config_file, "%s", fav_commands_path);
    fclose(config_file);

    FILE *fav_commands_file = fopen(fav_commands_path, "w");
    if (fav_commands_file == NULL) {
        fprintf(stderr, "Failed to create favorite commands file: %s\n", strerror(errno));
        return 1;
    }

    fclose(fav_commands_file);
    printf("Created favorite commands file\n");

    fav_commands.amount = 0;
    fav_commands.allocated = 1;
    fav_commands.list = malloc(sizeof(char *));
    fav_commands.list[0] = NULL;

    return 0;
}

int show_fav_commands(const char **) {
    if (fav_commands_path[0] == 0) {
        fprintf(stderr, "No favorite commands file found. Use \"favs crear|create <path>\" first.\n");
        return 1;
    }

    if (fav_commands.amount == 0) {
        fprintf(stderr, "No favorite commands found. Use \"favs cargar|load\" to load them.\n");
        return 0;
    }

    for (int i = 0; i < fav_commands.amount; i++)
        printf("%d : %s\n", i + 1, fav_commands.list[i]);

    return 0;
}

int reverse_comp(const void *a, const void *b) {
    return *(int *)b - *(int *)a;
}

int delete_fav_commands(const char **args) {
    if (fav_commands_path[0] == 0) {
        fprintf(stderr, "No favorite commands file found. Use \"favs crear|create <path>\" first.\n");
        return 1;
    }

    if (fav_commands.amount == 0) {
        fprintf(stderr, "No favorite commands found. Use \"favs cargar|load\" to load them.\n");
        return 0;
    }

    int indices_amount = 0;
    int indices[fav_commands.amount];

    while (*args != NULL) {
        char *end;
        const int index = strtol(*args, &end, 0);
        if (*end != 0) {
            fprintf(stderr, "Invalid number: %s\n", *args);
            return 1;
        }

        if (index < 1 || index > fav_commands.amount) {
            fprintf(stderr, "Id out of range: %d - last id is: %d\n", index, fav_commands.amount);
            return 1;
        }

        bool repeated = false;
        for (int i = 0; i < indices_amount; i++)
            if (indices[i] == index) {
                repeated = true;
                break;
            }

        if (repeated)
            continue;

        indices[indices_amount++] = index;
        args++;
    }

    if (indices_amount > 1)
        qsort(indices, indices_amount, sizeof(int), reverse_comp);

    printf("Deleted:\n");

    for (int i = 0; i < indices_amount; i++) {
        const int index = indices[i] - 1;

        printf("%d : %s\n", index + 1, fav_commands.list[index]);
        free(fav_commands.list[index]);

        for (int j = index; j < fav_commands.amount; j++)
            fav_commands.list[j] = fav_commands.list[j + 1];

        fav_commands.amount--;
    }

    return 0;
}

int search_fav_commands(const char **args) {
    if (fav_commands_path[0] == 0) {
        fprintf(stderr, "No favorite commands file found. Use \"favs crear|create <path>\" first.\n");
        return 1;
    }

    if (fav_commands.amount == 0) {
        fprintf(stderr, "No favorite commands found. Use \"favs cargar|load\" to load them.\n");
        return 0;
    }

    const char *search = args[0];
    if (search == NULL) {
        fprintf(stderr, "Usage: favs buscar|search <command>\n");
        return 1;
    }

    for (int i = 0; i < fav_commands.amount; i++)
        if (strstr(fav_commands.list[i], search) != NULL)
            printf("%d : %s\n", i, fav_commands.list[i]);

    return 0;
}

int clear_fav_commands(const char **) {
    if (fav_commands_path[0] == 0) {
        fprintf(stderr, "No favorite commands file found. Use \"favs crear|create <path>\" first.\n");
        return 1;
    }

    if (fav_commands.amount == 0) {
        fprintf(stderr, "No favorite commands found. Use \"favs cargar|load\" to load them.\n");
        return 0;
    }

    if (remove(fav_commands_path) != 0) {
        fprintf(stderr, "Failed to delete favorite commands file: %s\n", strerror(errno));
        return 1;
    }

    FILE *fav_commands_file = fopen(fav_commands_path, "w");
    if (fav_commands_file == NULL) {
        fprintf(stderr, "Failed to create favorite commands file: %s\n", strerror(errno));
        return 1;
    }

    fclose(fav_commands_file);
    free_fav_commands();

    printf("Cleared stored favorite commands file.\n");
    return 0;
}

int run_fav_commands(const char **args) {
    if (fav_commands_path[0] == 0) {
        fprintf(stderr, "No favorite commands file found. Use \"favs crear|create <path>\" first.\n");
        return 1;
    }

    if (fav_commands.amount == 0) {
        fprintf(stderr, "No favorite commands found. Use \"favs cargar|load\" to load them.\n");
        return 0;
    }

    const char *number_string = args[0];
    if (number_string == NULL) {
        fprintf(
            stderr,
            "Usage:\n\n"
            "favs ejecutar|run <number>\n"
            "favs <number> ejecutar|run\n"
        );
        return 1;
    }

    char *end;
    const int index = strtol(number_string, &end, 10);
    if (*end != 0) {
        fprintf(stderr, "Invalid number: %s\n", number_string);
        return 1;
    }

    if (index > fav_commands.amount) {
        fprintf(stderr, "No favorite command with id %d\n", index);
        return 1;
    }

    const char *command = fav_commands.list[index - 1];
    printf("%s\n", command);

    const ProcessArgs process_args = parse_args(command);
    execute_command(process_args.argc, process_args.argv);

    for (int i = 0; i < process_args.argc; i++)
        free(process_args.argv[i]);
    free(process_args.argv);

    return 0;
}

int load_fav_commands(const char **) {
    if (fav_commands_path[0] == 0) {
        fprintf(stderr, "No favorite commands file found. Use \"favs crear|create <path>\" first.\n");
        return 1;
    }

    free_fav_commands();

    FILE *fav_commands_file = fopen(fav_commands_path, "r");
    if (fav_commands_file == NULL) {
        fprintf(stderr, "Failed to open favorite commands file: %s\n", strerror(errno));
        return 1;
    }

    char c;
    int allocated_length = 256;
    char *line = malloc(allocated_length);
    line[0] = 0;

    int line_length = 0;

    while ((c = fgetc(fav_commands_file)) != EOF) {
        if (c != '\n') {
            if (line_length >= allocated_length) {
                allocated_length *= 2;

                char *reallocated = realloc(line, allocated_length);
                if (reallocated == NULL) {
                    free(line);
                    printf("\nOut of memory!\n");
                    exit(1);
                }

                line = reallocated;
            }

            line[line_length++] = c;
            continue;
        }

        if (line_length == 0)
            break;

        line[line_length] = 0;

        if (fav_commands.amount >= fav_commands.allocated) {
            fav_commands.allocated *= 2;

            char **reallocated = realloc(fav_commands.list, fav_commands.allocated * sizeof(char *));
            if (reallocated == NULL) {
                free(fav_commands.list);
                printf("\nOut of memory!\n");
                exit(1);
            }

            fav_commands.list = reallocated;
        }

        fav_commands.list[fav_commands.amount++] = strdup(line);
        printf("%d : %s\n", fav_commands.amount, line);

        line[0] = 0;
        line_length = 0;
    }

    if (fav_commands.amount == 0)
        printf("Favorite commands list empty.\n");

    printf("Loaded favorite commands.\n");
    return 0;
}

int save_fav_commands(const char **) {
    if (fav_commands_path[0] == 0) {
        fprintf(stderr, "No favorite commands file found. Use \"favs crear|create <path>\" first.\n");
        return 1;
    }

    FILE *fav_commands_file = fopen(fav_commands_path, "w");
    if (fav_commands_file == NULL) {
        fprintf(stderr, "Failed to open favorite commands file: %s\n", strerror(errno));
        return 1;
    }

    for (int i = 0; i < fav_commands.amount; i++)
        fprintf(fav_commands_file, "%s\n", fav_commands.list[i]);

    fclose(fav_commands_file);

    printf("Saved favorite commands.\n");
    return 0;
}
