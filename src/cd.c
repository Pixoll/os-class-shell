#include "cd.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

static char last_dir[PATH_MAX];  // initialized to zero

int exec_cd(const int argc, const char **argv) {
    if (last_dir[0] == 0)
        getcwd(last_dir, PATH_MAX);

    if (argc < 1) {
        fprintf(stderr, "cd: Usage: cd <path>\n");
        return 1;
    }

    const char *arg = argv[1];

    if (arg != NULL && strlen(arg) > PATH_MAX) {
        fprintf(stderr, "cd: Path is too long\n");
        return 1;
    }

    char current_dir[PATH_MAX];

    if (getcwd(current_dir, sizeof current_dir) == NULL) {
        /* current directory might be unreachable: not an error */
        *current_dir = 0;
    }

    if (arg == NULL) {
        arg = getenv("HOME");
    }

    if (!strcmp(arg, "-")) {
        if (*last_dir == 0) {
            fprintf(stderr, "cd: no previous directory\n");
            return 1;
        }
        arg = last_dir;
    } else {
        /* this should be done on all words during the parse phase */
        if (*arg == '~') {
            if (arg[1] == '/' || arg[1] == 0) {
                char path[PATH_MAX];
                snprintf(path, sizeof path, "%s%s", getenv("HOME"), arg + 1);
                arg = path;
            } else {
                /* ~name should expand to the home directory of user with login `name`
                   this can be implemented with getpwent() */
                fprintf(stderr, "cd: syntax not supported: %s\n", arg);
                return 1;
            }
        }
    }

    if (chdir(arg)) {
        fprintf(stderr, "cd: %s: %s\n", strerror(errno), arg);
        return 1;
    }

    strcpy(last_dir, current_dir);
    return 0;
}
