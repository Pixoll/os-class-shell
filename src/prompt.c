#include "prompt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif

static const char *home_path = NULL;
char hostname[HOST_NAME_MAX + 1];

char *replace_str(char *str, const char *orig, char *rep, int start);

void prompt() {
    if (home_path == NULL) {
        home_path = getenv("HOME");
        gethostname(hostname, sizeof(hostname));
    }

    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    replace_str(cwd, home_path, "~", 0);
    printf("\x1b[1;32m%s@%s\x1b[0m:\x1b[1;34m%s\x1b[0m$ ", getenv("USER"), hostname, cwd);
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
