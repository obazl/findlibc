#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>

#if INTERFACE
#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#endif

#include <unistd.h>

#include "liblogc.h"

#include "utarray.h"
#include "utstring.h"
#include "utstring.h"
#include "semver.h"
#include "findlibc.h"
/* #include "unity.h" */

#include "crawler_test.h"

#include "liblogc.h"

bool debug;
bool verbose;
int  verbosity;

UT_string *meta_path;

/* void print_usage(char *test); */
/* void set_options(char *test, struct option options[]); */
/* void initialize(char *test, int argc, char **argv); */

/* enum OPTS { */
/*     FLAG_HELP, */
/*     FLAG_DEBUG, */
/*     FLAG_DEBUG_CONFIG, */
/*     FLAG_DEBUG_SCM, */
/*     FLAG_DEBUG_SCM_LOADS, */

/*     FLAG_SHOW_CONFIG, */
/*     FLAG_TRACE, */
/*     FLAG_VERBOSE, */

/*     LAST */
/* }; */

/* struct option options[] = { */
/*     /\* 0 *\/ */
/*     [FLAG_DEBUG] = {.long_name="debug",.short_name='d', */
/*                     .flags=GOPT_ARGUMENT_FORBIDDEN | GOPT_REPEATABLE}, */
/*     [FLAG_DEBUG_CONFIG] = {.long_name="debug-config", */
/*                            .flags=GOPT_ARGUMENT_FORBIDDEN}, */
/*     [FLAG_DEBUG_SCM] = {.long_name="debug-scm", .short_name = 'D', */
/*                         .flags=GOPT_ARGUMENT_FORBIDDEN}, */
/*     [FLAG_DEBUG_SCM_LOADS] = {.long_name="debug-scm-loads", */
/*                               .flags=GOPT_ARGUMENT_FORBIDDEN}, */
/*     [FLAG_SHOW_CONFIG] = {.long_name="show-config", */
/*                           .flags=GOPT_ARGUMENT_FORBIDDEN}, */
/*     [FLAG_TRACE] = {.long_name="trace",.short_name='t', */
/*                     .flags=GOPT_ARGUMENT_FORBIDDEN}, */
/*     [FLAG_VERBOSE] = {.long_name="verbose",.short_name='v', */
/*                       .flags=GOPT_ARGUMENT_FORBIDDEN | GOPT_REPEATABLE}, */
/*     [FLAG_HELP] = {.long_name="help",.short_name='h', */
/*                    .flags=GOPT_ARGUMENT_FORBIDDEN}, */
/*     [LAST] = {.flags = GOPT_LAST} */
/* }; */

/* **************** **************** */
void pkg_handler(char *site_lib, char *pkg_dir, void *extra)
{
    (void)extra;
    if (verbose)
        log_debug("pkg_handler: %s", pkg_dir);
    /* log_debug("site-lib: %s", utstring_body(site_lib)); */

    utstring_renew(meta_path);
    utstring_printf(meta_path, "%s/%s/META",
                    site_lib,
                    pkg_dir);
                    /* utstring_body(opam_switch_lib), pkg_name); */
    if (verbose)
        log_info("\tmeta_path: %s", utstring_body(meta_path));

    errno = 0;
    if ( access(utstring_body(meta_path), F_OK) == 0 ) {
        /* exists */
        /* log_info("accessible: %s", utstring_body(meta_path)); */

        struct obzl_meta_package *pkg
            = obzl_meta_parse_file(utstring_body(meta_path));
        if (pkg == NULL) {
            if (errno == -1) {
#if defined(TRACING)
                log_warn("Empty META file: %s", utstring_body(meta_path));
#endif
                /* check dune-package for installed executables */
                /* chdir(old_cwd); */
                return;
            } else
                if (errno == -2)
                    log_warn("META file contains only whitespace: %s", utstring_body(meta_path));
                else
                    log_error("Error parsing %s", utstring_body(meta_path));
        } else {
/* #if defined(DEVBUILD) */
            if (verbose)
                log_info("PARSED %s", utstring_body(meta_path));
            /* if (mibl_debug_findlib) */
                /* DUMP_PKG(0, pkg); */
/* #endif */
        }
    } else {
        /* fail */
        /* perror(utstring_body(meta_path)); */
/* #if defined(TRACING) */
        /* if (mibl_debug) */
            log_warn("%s: %s", strerror(errno), utstring_body(meta_path));
/* #endif */
        /* chdir(old_cwd); */
        /* return; */
        /* exit(EXIT_FAILURE); */
    }
}

int main(int argc, char *argv[])
{
    int opt;

    UT_string *findlib_site_lib;
    utstring_new(findlib_site_lib);

    utstring_new(meta_path);
    char *opam_switch = NULL;

    char *homedir = getenv("HOME");

    while ((opt = getopt(argc, argv, "ds:hv")) != -1) {
        switch (opt) {
        case 'd':
            debug = true;
            break;
        case 'v':
            verbose = true;
            break;
        case 's':
            /* BUILD.bazel or BUILD file */
            opam_switch = optarg;
            utstring_printf(findlib_site_lib,
                            "%s/.opam/%s/lib",
                            homedir,
                            optarg);
            break;
        case 'h':
            log_info("Help: ");
            exit(EXIT_SUCCESS);
        default:
            log_error("Usage: %s -s <opam switch>", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (opam_switch == NULL) {
        log_error("-s <opam switch> must be provided.");
        exit(EXIT_FAILURE);
    } else {
        log_info("findlib site-lib: %s", utstring_body(findlib_site_lib));
    }

    char *wd = getenv("BUILD_WORKING_DIRECTORY");
    if (wd) {
        /* we launched from bazel workspace, cd to launch dir */
        chdir(wd);
    }

    UT_array *opam_include_pkgs;
    utarray_new(opam_include_pkgs,&ut_str_icd);

    UT_array *opam_exclude_pkgs;
    utarray_new(opam_exclude_pkgs,&ut_str_icd);


    findlib_map(opam_include_pkgs,
                opam_exclude_pkgs,
                utstring_body(findlib_site_lib),
                pkg_handler,
                NULL);

    utstring_free(findlib_site_lib);
    utstring_free(meta_path);
    log_info("crawler_test exiting");
}
