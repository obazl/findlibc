#include <assert.h>
#include <ctype.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#if EXPORT_INTERFACE
#include <fts.h>
#endif
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if INTERFACE
#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#endif

/* #if INTERFACE */
#include "utarray.h"
#include "utstring.h"
/* #endif */

#include "log.h"

/* #include "libfindlib.h" */
#include "handlers_findlib.h"

/* we emit one ws file per opam (findlib) pkg */
UT_string *workspace_file;

UT_string *meta_path; /* = opam_switch_lib + pkg_name */

/* for emitting:  */
UT_string *pkg_parent;
UT_string *imports_path;
char *ocaml_ws = "ocaml";
extern char *bazel_ws_root;

bool g_ppx_pkg = false;
/* extern bool stdlib_root = false; */
bool emitted_bootstrapper = false;

UT_string *workspace_file;
UT_string *pkg_parent;

UT_string *bzl_switch_pfx;      /* emit output locn */

UT_string *bazel_pkg_root;
UT_string *build_bazel_file;

extern int errnum;

#if EXPORT_INTERFACE
#include <stdbool.h>
struct logging {
    int verbosity;
    int log_level;
    int parse_verbosity;
    int parse_log_level;
    int lex_verbosity;
    int lex_log_level;
    bool quiet;
    bool log_color;
};
#endif

EXPORT struct logging logger;

bool _skip_pkg(char *pkg)
{
    log_debug("_skip_pkg: %s", pkg);

    static const char * skips[] = {
        "lib/bigarray/META",
        "lib/compiler-libs/META",
        "lib/dynlink/META",
        "lib/num/META",
        "lib/ocamldoc/META",
        "lib/stdlib/META",
        "lib/str/META",
        "lib/threads/META",
        "lib/unix/META",
        "lib/raw_spacetime/META",
    };

#define skips_ct (sizeof (skips) / sizeof (const char *))

    int len = strlen(pkg);
    int skiplen;

    for (unsigned long i = 0; i < skips_ct; i++) {
        skiplen = strlen(skips[i]);
        if (strncmp(pkg + len - skiplen,
                    skips[i], skiplen) == 0) {
            log_warn("SKIPPING %s", skips[i]);
            return true;
        }
    }


    /* avoid matching ocaml-compiler-libs */
    if (strncmp(pkg + len - 22, "lib/compiler-libs/META", 16) == 0) {
        log_warn("SKIPPING compiler-libs/META");
        return true;
    }

    if (strncmp(pkg + len - 16, "lib/dynlink/META", 16) == 0) {
        log_warn("SKIPPING dynlink/META");
        return true;
    }
    if (strncmp(pkg + len - 22, "lib/ocamldoc/META", 22) == 0) {
        log_warn("SKIPPING ocamldoc/META");
        return true;
    }
    if (strncmp(pkg + len - 21, "lib/threads/META", 21) == 0) {
        log_warn("SKIPPING threads/META");
        return true;
    }
    if (strncmp(pkg + len - 18, "lib/unix/META", 18) == 0) {
        log_warn("SKIPPING unix/META");
        return true;
    }

    // version-dependent: bigarray, num, raw_spacetime

    // Bigarray moved to standard lib in v. 4.07
    // so no need to list as explicit dep?
    if (strncmp(pkg + len - 22, "lib/bigarray/META", 22) == 0) {
        log_warn("SKIPPING bigarray/META");
        return true;
    }

    // raw_spacetime - removed in v. 4.12(?)
    if (strncmp(pkg + len - 27, "lib/raw_spacetime/META", 27) == 0) {
        log_warn("SKIPPING raw_spacetime/META");
        return true;
    }

    // num - moved from core to separate lib in v. 4.06.0
    // skip if version < 4.06 ?
    // however, opam seems to install num* in lib/ocaml "for backward compatibility", and installs lib/num/META.
    /* "New applications that need arbitrary-precision arithmetic should use the Zarith library (https://github.com/ocaml/Zarith) instead of the Num library, and older applications that already use Num are encouraged to switch to Zarith." */
    // https://www.dra27.uk/blog/platform/2018/01/31/num-system.html
    if (strncmp(pkg + len - 17, "lib/num/META", 17) == 0) {
        log_warn("SKIPPING num/META");
        return true;
    }

    /* TMP HACK: skip some pkgs for which we use template BUILD files */
    /* if (strncmp(pkg + len - 13, "digestif/META", 13) == 0) { */
    /*     log_warn("SKIPPING digestif/META"); */
    /*     return true; */
    /* } */

    /* if (strncmp(pkg + len - 13, "ptime/META", 10) == 0) { */
    /*     log_warn("SKIPPING ptime/META"); */
    /*     return true; */
    /* } */

    return false;
}

/*
  switch_lib: abs path to switch e.g. $HOME/.opam/<switch>/lib
  obazl_opam_root: coswitch output path, e.g. $XDG_DATA_HOME/obazl/opam/<switch>/lib
  pkgdir: global, pkg name
  metafile: 'META'
 */
/* int handle_findlib_meta(char *switch_lib, */
/*                         char *obazl_opam_root, */
/*                         char *pkgdir, */
/*                         char *metafile) */
EXPORT int handle_findlib_meta(FTSENT *ftsentry) /* OBSOLETE */
{
#if defined(TRACING)
    log_debug(BLU "handle_findlib_meta" CRESET);
    log_info("%-20s%s", "ftsentry->name:", ftsentry->fts_name);
    log_info("%-20s%s", "ftsentry->path:", ftsentry->fts_path);
    log_info("%-20s%s", "ftsentry->accpath:", ftsentry->fts_accpath);
    /* log_debug("  switch_lib: %s; obazl_opam_root: %s; pkgdir: %s; metafile: %s", */
    /*           switch_lib, obazl_opam_root, pkgdir, metafile); */
#endif

    /* fflush(bootstrap_FILE); */

    UT_string *buf;
    utstring_new(buf);
    utstring_printf(buf, "%s", ftsentry->fts_path);
    /* utstring_printf(buf, "%s/%s/%s", switch_lib, pkgdir, metafile); */

    /* mkdir_r(buf, "/"); */
    /* mystrcat(buf, "/BUILD.bazel"); */

    /* log_debug("out buf: %s", buf); */
    /* FILE *f; */
    /* if ((f = fopen(buf, "w")) == NULL){ */
    /*     log_fatal("Error! opening file %s", buf); */
    /*     exit(EXIT_FAILURE); */
    /* } */
    /* fprintf(f, "## test\n"); //  "src: %s/%s\n", pkgdir, metafile); */
    /* fclose(f); */

    errno = 0;
#if defined(TRACING)
    log_debug("PARSING: %s", utstring_body(buf));
#endif
    struct obzl_meta_package *pkg = obzl_meta_parse_file(utstring_body(buf));
    if (pkg == NULL) {
        if (errno == -1)
            log_info("Empty META file: %s", utstring_body(buf));
        else
            if (errno == -2)
                log_info("META file contains only whitespace: %s", utstring_body(buf));
            else
                log_error("Error parsing %s", utstring_body(buf));
        emitted_bootstrapper = false;
    } else {
#if defined(TRACING)
        log_warn("PARSED %s", utstring_body(buf));
        if (mibl_debug_findlib)
            dump_package(0, pkg);
#endif

        return 0;

        /* stdlib_root = false; */

        if (obzl_meta_entries_property(pkg->entries, "library_kind")) {
            /* special handling for ppx packages */
            log_debug("handling ppx package: %s", obzl_meta_package_name(pkg));
            g_ppx_pkg = true;
            /* emit_build_bazel_ppx(obazl_opam_root, host_repo, "lib", "", pkg); */
        } else {
            log_debug("handling normal package: %s", obzl_meta_package_name(pkg));
            g_ppx_pkg = true;
        }

        /* skip ocaml core pkgs, we do them by hand */
        if (_skip_pkg(utstring_body(buf))) {
            log_debug("SKIPPING pkg %s", utstring_body(buf));
            return 0;
        }

        /* emit_new_local_pkg_repo(bootstrap_FILE, /\* _pkg_prefix, *\/ */
        /*                         pkg); */
        /* fflush(bootstrap_FILE); */
        /* emit_local_repo_decl(bootstrap_FILE, pkg->name); */
        emitted_bootstrapper = true;

        /* fflush(bootstrap_FILE); */

        UT_string *imports_path;
        utstring_new(imports_path);
        /* utstring_printf(imports_path, "_lib/%s", */
        utstring_printf(imports_path, "%s",
                        obzl_meta_package_name(pkg));
        log_debug("emitting buildfile for pkg: %s", pkg->name);
        /* dump_package(0, pkg); */
        /* fflush(bootstrap_FILE); */

        utstring_renew(workspace_file);
        /* utstring_printf(workspace_file, "%s/%s", obazl_opam_root, pkg->name); */
        mkdir_r(utstring_body(workspace_file));
        utstring_printf(workspace_file, "%s", "/WORKSPACE.bazel");
        /* printf("emitting ws file: %s\n", utstring_body(workspace_file)); */
        /* emit_workspace_file(workspace_file, pkg->name); */

        utstring_renew(pkg_parent);
        /* emit_build_bazel(host_repo, */
        /*                  obazl_opam_root,      /\* _repo_root: "." or "./tmp/opam" *\/ */
        /*                  0,                    /\* level *\/ */
        /*                  pkg->name, /\* pkg root - constant *\/ */
        /*                  pkg_parent, */
        /*                  NULL, // "buildfiles",        /\* _pkg_prefix *\/ */
        /*                  utstring_body(imports_path), */
        /*                 /\* "",      /\\* pkg-path *\\/ *\/ */
        /*                  pkg); */
        /* fflush(bootstrap_FILE); */
        log_debug("emittED buildfile for pkg: %s", pkg->name);
    }
    /* free pkg?? */
    /* dune-package file */
    /* UT_string *dune_pkg_file; */
    /* utstring_new(dune_pkg_file); */
    /* utstring_printf(dune_pkg_file, "%s/%s/dune-package", */
    /*                 switch_lib, pkgdir); */
    /* printf("\nCHECKING DUNE-PACKAGE: %s\n", utstring_body(dune_pkg_file)); */
    /* if (access(utstring_body(dune_pkg_file), F_OK) == 0) { */
    /*     printf("obazl dir: %s\n", obazl_opam_root); */
    /*     emit_opam_pkg_bindir(dune_pkg_file, switch_lib, pkgdir, obazl_opam_root, emitted_bootstrapper); */
    /* } */
    /* log_debug("FINISHED handling:  switch_lib: %s; obazl_opam_root: %s; pkgdir: %s; metafile: %s", */
    /*           switch_lib, obazl_opam_root, pkgdir, metafile); */
    return 0;
}

void handle_findlib_pkg(// char *opam_switch_lib,
                        UT_string *opam_switch_lib,
                        char *pkg_name,
                        UT_array *opam_pending_deps,
                        UT_array *opam_completed_deps)
{
#if defined(TRACING)
    log_trace(RED "handle_findlib_pkg:" CRESET " %s", pkg_name);
    log_trace("global opam_lib: %s", utstring_body(opam_switch_lib));
#endif

    utstring_new(build_bazel_file);
    utstring_new(bazel_pkg_root);

    utstring_new(bzl_switch_pfx);
    utstring_concat(bzl_switch_pfx, opam_switch_lib);

    char *old_cwd = getcwd(NULL, 0);

    int rc = chdir(utstring_body(opam_switch_lib));
    (void)rc;

#if defined(TRACING)
    if (mibl_debug) log_debug("%-16s%s", "cwd:",  getcwd(NULL, 0));
#endif

    /* FTS* tree = NULL; */
    /* FTSENT *ftsentry     = NULL; */

    errno = 0;

    utstring_renew(meta_path);
    utstring_printf(meta_path, "%s/%s/META",
                    utstring_body(opam_switch_lib), pkg_name);

    /* log_info("meta_path: %s", utstring_body(meta_path)); */

    errno = 0;
    if ( access(utstring_body(meta_path), F_OK) == 0 ) {
        /* exists */
        /* log_info("accessible: %s", utstring_body(meta_path)); */
    } else {
        /* fail */
        /* perror(utstring_body(meta_path)); */
#if defined(TRACING)
        if (mibl_debug)
            log_info("%s: %s", strerror(errno), utstring_body(meta_path));
#endif
        chdir(old_cwd);
        return;
        /* exit(EXIT_FAILURE); */
    }

    errno = 0;
#if defined(TRACING)
    log_debug("PARSING: %s", utstring_body(meta_path));
#endif
    struct obzl_meta_package *pkg = obzl_meta_parse_file(utstring_body(meta_path));
    if (pkg == NULL) {
        if (errno == -1) {
#if defined(TRACING)
            log_warn("Empty META file: %s", utstring_body(meta_path));
#endif
            /* check dune-package for installed executables */
            chdir(old_cwd);
            return;
        } else
            if (errno == -2)
                log_warn("META file contains only whitespace: %s", utstring_body(meta_path));
            else
                log_error("Error parsing %s", utstring_body(meta_path));
        /* emitted_bootstrapper = false; */
    } else {
#if defined(TRACING)
        if (mibl_debug)
            log_warn("PARSED %s", utstring_body(meta_path));
        if (mibl_debug_findlib)
            dump_package(0, pkg);
#endif
    }

    /* emit ws file */
    utstring_renew(workspace_file);
    utstring_printf(workspace_file, "%s/%s/WORKSPACE.bazel",
                    utstring_body(opam_switch_lib), pkg_name);
    /* we know META file exists, so pkg path exists */
    emit_workspace_file(workspace_file, pkg->name);

    /* emit buildfile(s) */

    /* UT_string *imports_path; */
    utstring_new(imports_path);
    utstring_printf(imports_path, "%s",
                    obzl_meta_package_name(pkg));

    utstring_new(pkg_parent);
    /* utarray_new(_deps,&ut_str_icd); */
    /* _deps = pkg_deps(pkg, _deps); */
    /* char **p = NULL; */
    /* while ( (p=(char**)utarray_next(_deps, p))) { */
    /*     log_debug("dep: %s",*p); */
    /* } */

    emit_build_bazel(// ocaml_ws,       /* "ocaml" */
                     bazel_ws_root, /* same as origin (opam_lib) */
                     0,               /* level */
                     pkg->name,
                     pkg_parent, /* needed for handling subpkgs */
                     NULL, // "buildfiles",        /* _pkg_prefix */
                     utstring_body(imports_path),
                     /* "",      /\* pkg-path *\/ */
                     pkg,
                     opam_pending_deps,
                     opam_completed_deps);
    chdir(old_cwd);
    return;
}
