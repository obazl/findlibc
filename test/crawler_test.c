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

#include "findlibc.h"
#include "crawler_test.h"

bool verbose;
int  verbosity;

UT_string *meta_path;

void pkg_handler(char *switch_pfx,
                 char *site_lib, /* switch_lib */
                 char *pkg_dir,  /* subdir of site_lib */
                 void *_paths) /* struct paths_s* */
{
    (void)_paths; /* not needed for testing */
    /* bool empty_pkg = false; */
    /* (void)empty_pkg; */
    (void)switch_pfx;
    /* UT_string *registry = (UT_string*)paths->registry; */
    /* UT_string *coswitch_lib = (UT_string*)paths->coswitch_lib; */
    /* struct obzl_meta_package *pkgs */
    /*     = (struct obzl_meta_package*)paths->pkgs; */

    if (verbosity > 1) {
        log_debug("pkg_handler: %s", pkg_dir);
        log_debug("site-lib: %s", site_lib);
        /* log_debug("registry: %s", utstring_body(registry)); */
        /* log_debug("coswitch: %s", utstring_body(coswitch_lib)); */
        /* log_debug("pkgs ct: %d", HASH_COUNT(pkgs)); */
    }

    utstring_renew(meta_path);
    utstring_printf(meta_path, "%s/%s/META",
                    site_lib,
                    pkg_dir);
                    /* utstring_body(opam_switch_lib), pkg_name); */
    if (verbosity > 1)
        log_info("meta_path: %s", utstring_body(meta_path));

    errno = 0;
    if ( access(utstring_body(meta_path), F_OK) != 0 ) {
        // no META happens for e.g. <switch>/lib/stublibs
        /* log_warn("%s: %s", */
        /*          strerror(errno), utstring_body(meta_path)); */
        return;
    /* } else { */
    /*     /\* exists *\/ */
    /*     log_info("accessible: %s", utstring_body(meta_path)); */
    }
    /* empty_pkg = false; */
    errno = 0;
    // pkg must be freed...
    struct obzl_meta_package *pkg
        = obzl_meta_parse_file(utstring_body(meta_path));

    if (pkg == NULL) {
        if ((errno == -1)
            || (errno == -2)) {
            /* empty_pkg = true; */
            /* #if defined(TRACING) */
            if (verbosity > 2)
                log_warn("Empty META file: %s", utstring_body(meta_path));
            return;
        } else {
            log_error("Error parsing %s", utstring_body(meta_path));
            return;
        }
    } else {
        /* #if defined(PROFILE_fastbuild) */
        if (verbosity > 2) {
            log_info("Parsed %d %s", verbosity, utstring_body(meta_path));
        }
    }

    char *pkg_name = strdup(pkg->name);
    log_debug("pkg_name: %s", pkg_name);
}

int main(int argc, char *argv[])
{
    int opt;

    /* verbosity = 3; */

    UT_string *switch_pfx;
    utstring_new(switch_pfx);

    UT_string *findlib_site_lib;
    utstring_new(findlib_site_lib);

    utstring_new(meta_path);
    char *opam_switch;

    char *homedir = getenv("HOME");

    while ((opt = getopt(argc, argv, "s:hv")) != -1) {
        switch (opt) {
        case 's':
            /* BUILD.bazel or BUILD file */
            opam_switch = optarg;
            utstring_printf(switch_pfx,
                            "%s/.opam/%s",
                            homedir,
                            optarg);
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
                utstring_body(switch_pfx),
                utstring_body(findlib_site_lib),
                pkg_handler,
                NULL);
                /* pkg_handler); */

    utstring_free(findlib_site_lib);
    utstring_free(meta_path);
    log_info("crawler_test exiting");
}
