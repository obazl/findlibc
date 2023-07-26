#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <glob.h>
#include <libgen.h>
#ifdef __linux__
#include <linux/limits.h>
#include <sys/wait.h>           /* waitpid */
#else
#include <limits.h>             /* PATH_MAX */
#endif
#include <pwd.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

/* #include "ini.h" */
#include "log.h"
/* #if EXPORT_INTERFACE */
/* #include "utarray.h" */
#include "utstring.h"
/* #endif */

#include "utils.h"

int strsort(const void *_a, const void *_b)
{
    const char *a = *(const char* const *)_a;
    const char *b = *(const char* const *)_b;
    return strcmp(a,b);
}

void mkdir_r(const char *dir) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    mkdir(tmp, S_IRWXU);
}

/* LOCAL char principal[256]; */

/* LOCAL char *_module_name(FTSENT *ftsentry, char *ext) */
/* { */
/*     strlcpy(principal, ftsentry->fts_name, 256); */
/*     principal[ext - ftsentry->fts_name] = '\0'; */
/*     principal[0] = toupper(principal[0]); */
/*     return (char *)principal; */
/* } */

/* LOCAL char *_principal_name(FTSENT *ftsentry, char *ext) */
/* { */
/*     strlcpy(principal, ftsentry->fts_name, 256); */
/*     principal[ext - ftsentry->fts_name] = '\0'; */
/*     /\* principal[0] = toupper(principal[0]); *\/ */
/*     return (char *)principal; */
/* } */

EXPORT int copyfile(char *fromfile, char *tofile) {
    char ch;// source_file[20], target_file[20];

    FILE *source = fopen(fromfile, "r");
    if (source == NULL) {
        fprintf(stderr, "copyfile fopen fail on fromfile: %s\n", fromfile);
        exit(EXIT_FAILURE);
    }
    FILE *target = fopen(tofile, "w");
    if (target == NULL) {
        fclose(source);
        fprintf(stderr, "copyfile fopen fail on tofile: %s\n", tofile);
        exit(EXIT_FAILURE);
    }
    while ((ch = fgetc(source)) != EOF)
        fputc(ch, target);
/* #if defined(TRACING) */
/*         printf("File copy successful: %s -> %s.\n", */
/*                fromfile, tofile); */
/* #endif */
    fclose(source);
    fclose(target);
    return 0;
}
