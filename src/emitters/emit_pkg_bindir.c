/* #include <assert.h> */
#include <errno.h>
/* #include <dirent.h> */
/* #include <fnmatch.h> */
/* #include <libgen.h> */
/* #include <regex.h> */

/* #if EXPORT_INTERFACE */
/* #include <stdio.h> */
/* #endif */

/* #ifdef __linux__ */
/* #include <linux/limits.h> */
/* #else */
/* #include <limits.h> */
/* #endif */
/* #include <string.h> */
/* #include <sys/stat.h> */
#include <unistd.h>             /* access */

/* #if EXPORT_INTERFACE */
/* #include "s7.h" */
/* #endif */

#include "utarray.h"
#include "utstring.h"

#include "libs7.h"

#include "log.h"

#include "utils.h"
#include "findlib.h"

#include "emit_pkg_bindir.h"

extern UT_string *opam_switch_lib;
extern UT_string *opam_coswitch_lib;

int errnum;

#define TO_STR(x) s7_object_to_c_string(s7, x)

/* **************************************************************** */
/* s7_scheme *s7;                  /\* GLOBAL s7 *\/ */
/* /\* const char *errmsg = NULL; *\/ */

/* static int level = 0; */
extern int spfactor;
extern char *sp;

#if defined(TRACING)
extern int indent;
extern int delta;
#endif

bool stdlib_root = false;

/* char *buildfile_prefix = "@//" HERE_OBAZL_ROOT "/buildfiles"; */
/* was: "@//.opam.d/buildfiles"; */

long *KPM_TABLE;

FILE *opam_resolver;

extern UT_string *repo_name;

/* from dune_readers.c */
UT_array *get_pkg_executables(s7_scheme *s7,
                              s7_pointer _stanzas)
/* UT_string *dune_pkg_file) */
{
    /* TRACE_ENTRY(get_pkg_executables); */

    s7_pointer stanzas = (s7_pointer) _stanzas;
    UT_string *outpath;
    UT_string *opam_bin;
    utstring_new(outpath);
    utstring_new(opam_bin);

    UT_array *bins;
    utarray_new(bins, &ut_str_icd);

    /* s7_pointer stanzas = read_dune_package(dune_pkg_file); */

    s7_pointer iter, binfile;

    s7_pointer e = s7_inlet(s7,
                            s7_list(s7, 1,
                                    s7_cons(s7,
                                            s7_make_symbol(s7, "stanzas"),
                                            stanzas)));

    char * exec_sexp =
        "(let ((files (assoc 'files (cdr stanzas))))"
        "  (if files"
        "      (let ((bin (assoc 'bin (cdr files))))"
        "          (if bin (cadr bin)))))";

    s7_pointer executables = s7_eval_c_string_with_environment(s7, exec_sexp, e);

    if (executables == s7_unspecified(s7))
        return bins;

#if defined(DEVBUILD)
    if (mibl_debug) {
        /* log_debug("Pkg: %s", utstring_body(dune_pkg_file)); */
        LOG_S7_DEBUG("executables", executables);
    }
#endif

    /* /\* result is list of executables installed in $PREFIX/bin *\/ */
    /* if (s7_is_list(s7, executables)) { */
    /*     if (verbose) { */
    /*     } */
    /* } */
    iter = s7_make_iterator(s7, executables);
        //gc_loc = s7_gc_protect(s7, iter);
    if (!s7_is_iterator(iter)) {
        log_error("s7_make_iterator failed");
#if defined(DEVBUILD)
        LOG_S7_DEBUG("not an iterator", iter);
#endif
    }
    if (s7_iterator_is_at_end(s7, iter)) {
        log_error("s7_iterator_is_at_end prematurely");
#if defined(DEVBUILD)
        LOG_S7_DEBUG("iterator prematurely done", iter);
#endif
    }
    char *f;
    while (true) {
        binfile = s7_iterate(s7, iter);
        if (s7_iterator_is_at_end(s7, iter)) break;
#if defined(DEVBUILD)
        LOG_S7_DEBUG("binfile", binfile);
#endif
        f = TO_STR(binfile);
        utarray_push_back(bins, &f);
        free(f);
    }
        /* utstring_renew(opam_bin); */
        /* utstring_printf(opam_bin, "%s/%s", */
        /*                 utstring_body(opam_switch_bin), */
        /*                 TO_STR(binfile)); */

        /* utstring_renew(outpath); */
        /* utstring_printf(outpath, "%s/%s/bin/%s", */
        /*                 obazl, pkg, TO_STR(binfile)); */
        /* rc = symlink(utstring_body(opam_bin), */
        /*              utstring_body(outpath)); */
        /* if (rc != 0) { */
        /*     if (errno != EEXIST) { */
        /*         perror(NULL); */
        /*         fprintf(stderr, "exiting\n"); */
        /*         exit(EXIT_FAILURE); */
        /*     } */
        /* } */
        /* if (!emitted_bootstrapper) */
        /*     emit_local_repo_decl(bootstrap_FILE, pkg); */

        /* fprintf(ostream, "exports_files([\"%s\"])\n", TO_STR(binfile)); */
        /* fprintf(ostream, "## src: %s\n", utstring_body(opam_bin)); */
        /* fprintf(ostream, "## dst: %s\n", utstring_body(outpath)); */
    /* } */
    return bins;
}

/* from dune_readers.c: */
UT_array *get_pkg_stublibs(s7_scheme *s7,
                           char *pkg, void *_stanzas)
/* UT_string *dune_pkg_file) */
{
    /* TRACE_ENTRY(get_pkg_stublibs); */
    (void)pkg;

    s7_pointer stanzas = (s7_pointer) _stanzas;
/* #if defined(DEVBUILD) */
/*     log_debug("stanzas: %s", TO_STR(stanzas)); */
/* #endif */

    UT_string *outpath;
    UT_string *opam_bin;
    utstring_new(outpath);
    utstring_new(opam_bin);

    UT_array *stubs;
    utarray_new(stubs, &ut_str_icd);

    /* s7_pointer stanzas = read_dune_package(dune_pkg_file); */

    s7_pointer iter, stublib_file;

    s7_pointer e = s7_inlet(s7,
                            s7_list(s7, 1,
                                    s7_cons(s7,
                                            s7_make_symbol(s7, "stanzas"),
                                            stanzas)));

    char * stublibs_sexp =
        "(let ((files (assoc 'files (cdr stanzas))))"
        "  (if files"
        "      (let ((bin (assoc 'stublibs (cdr files))))"
        "          (if bin (cadr bin)))))";

    s7_pointer stublibs = s7_eval_c_string_with_environment(s7, stublibs_sexp, e);

    if (stublibs == s7_unspecified(s7))
        return stubs;

#if defined(DEVBUILD)
    if (mibl_debug) {
        /* log_debug("Pkg: %s", utstring_body(dune_pkg_file)); */
        LOG_S7_DEBUG(RED "STUBLIBS" CRESET, stublibs);
    }
#endif

    /* result is list of stublibs installed in $PREFIX/bin */
    /* if (s7_is_list(s7, stublibs)) { */
    /*     if (verbose) { */
    /*         log_info(GRN "%s stublibs:" CRESET " %s", */
    /*                  pkg, */
    /*                  /\* " for %s: %s\n", *\/ */
    /*                  /\* utstring_body(dune_pkg_file), *\/ */
    /*                  TO_STR(stublibs)); */
    /*     } */
    /* } */
    iter = s7_make_iterator(s7, stublibs);
        //gc_loc = s7_gc_protect(s7, iter);
    if (!s7_is_iterator(iter)) {
        log_error("s7_is_iterator fail");
#if defined(DEVBUILD)
        LOG_S7_DEBUG("not an iterator", iter);
#endif
    }
    if (s7_iterator_is_at_end(s7, iter)) {
        log_error("s7_iterator_is_at_end prematurely");
#if defined(DEVBUILD)
        LOG_S7_DEBUG("iterator prematurely done", iter);
#endif
    }
    char *f;
    while (true) {
        stublib_file = s7_iterate(s7, iter);
        if (s7_iterator_is_at_end(s7, iter)) break;
#if defined(DEVBUILD)
        LOG_S7_DEBUG("stublib_file", stublib_file);
#endif
        f = TO_STR(stublib_file);
        utarray_push_back(stubs, &f);
        free(f);
    }
    return stubs;
}

/* FIXME: same in mibl/error_handler.c */
LOCAL void emit_opam_pkg_bindir(s7_scheme *s7,
                                const char *pkg) // UT_string *dune_pkg_file)
                          /* char *switch_lib, */
                          /* char *pkg, */
                          /* char *obazl, */
                          /* bool emitted_bootstrapper) */
{
#if defined(TRACING)
    if (mibl_trace) log_trace("emit_opam_pkg_bindir");
#endif

    /* read dune-package file. if it exports executables:
       1. write bin/BUILD.bazel with a rule for each
       2. symlink from opam switch
     */

    utstring_renew(dune_pkg_file);
    utstring_printf(dune_pkg_file, "%s/%s/dune-package",
                    utstring_body(opam_switch_lib), /* global */
                    pkg);

    //FIXME: use read_dunefile, delete read_dune_package
    //void *stanzas = read_dune_package(dune_pkg_file);
    s7_pointer stanzas = s7_call(s7,
                                 s7_name_to_value(s7, "dune:read"),
                                 s7_nil(s7));
    /* s7_pointer executables = get_pkg_executables(dune_pkg_file); */
    UT_array *executables = get_pkg_executables(s7, stanzas);

    if (utarray_len(executables) == 0) goto stublibs;

    /* if executables not null:
       1. create 'bin' subdir of pkg
       2. add WORKSPACE.bazel to <pkg>/bin
       3. symlink executables to <pkg>/bin
       4. add BUILD.bazel with exports_files for linked executables
     */

    UT_string *outpath;
    utstring_new(outpath);
    UT_string *opam_bin;
    utstring_new(opam_bin);
    /* s7_pointer iter, binfile; */

    /* for most pkgs, WORKSPACE.bazel is already written,
       but for some containing only executables (e.g. menhir),
       we need to write it now
    */
    utstring_renew(outpath);
    utstring_printf(outpath, "%s/%s/WORKSPACE.bazel",
                    utstring_body(opam_coswitch_lib),
                    pkg);
#if defined(TRACING)
    if (mibl_debug)
        log_debug("checking ws: %s", utstring_body(outpath));
#endif
    if (access(utstring_body(outpath), F_OK) != 0) {
#if defined(TRACING)
        log_debug("creating %s\n", utstring_body(outpath));
#endif
        /* if obazl/pkg not exist, create it with WORKSPACE */
        /* utstring_renew(outpath); */
        /* utstring_printf(outpath, "%s/%s", */
        /*                 utstring_body(opam_coswitch_lib), */
        /*                 pkg); */
        /* errno = 0; */
        /* if (access(utstring_body(outpath), F_OK) != 0) { */
        /*     utstring_printf(outpath, "/%s", "WORKSPACE.bazel"); */
        emit_workspace_file(outpath, pkg);
    }
    errno = 0;
    utstring_renew(outpath);
    utstring_printf(outpath, "%s/%s/bin",
                    utstring_body(opam_coswitch_lib),
                    //rootws, /* obazl, */
                    pkg);
    mkdir_r(utstring_body(outpath));

    /* create <pkg>/bin/BUILD.bazel */
    utstring_renew(outpath);
    utstring_printf(outpath, "%s/%s/bin/BUILD.bazel",
                    utstring_body(opam_coswitch_lib),
                    pkg);
    /* rc = access(utstring_body(build_bazel_file), F_OK); */
#if defined(TRACING)
    log_debug("fopening: %s", utstring_body(outpath));
#endif

    FILE *ostream;
    ostream = fopen(utstring_body(outpath), "w");
    if (ostream == NULL) {
        printf(RED "ERROR" CRESET "fopen failure for %s", utstring_body(outpath));
        log_error("fopen failure for %s", utstring_body(outpath));
        perror(utstring_body(outpath));
        log_error("Value of errno: %d", errnum);
        log_error("fopen error %s", strerror( errnum ));
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "## generated file - DO NOT EDIT\n");

    fprintf(ostream, "exports_files([");

    /* For each executable, create symlink and exports_files entry */
    char **p = NULL;
    while ( (p=(char**)utarray_next(executables,p))) {
#if defined(TRACING)
        log_debug("bin: %s",*p);
#endif
        fprintf(ostream, "\"%s\",", *p);
        /* symlink */
        utstring_renew(opam_bin);
        utstring_printf(opam_bin, "%s/%s",
                        utstring_body(opam_switch_bin),
                        *p);
#if defined(TRACING)
        log_debug("SYMLINK SRC: %s", utstring_body(opam_bin));
#endif
        utstring_renew(outpath);
        utstring_printf(outpath, "%s/%s/bin/%s",
                        utstring_body(opam_coswitch_lib),
                        pkg,
                        *p);
#if defined(TRACING)
        log_debug("SYMLINK DST: %s", utstring_body(outpath));
#endif
        int rc = symlink(utstring_body(opam_bin),
                     utstring_body(outpath));
        symlink_ct++;
        if (rc != 0) {
            if (errno != EEXIST) {
                perror(NULL);
                fprintf(stderr, "exiting\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    fprintf(ostream, "])\n");
    fclose(ostream);

    if (verbose && verbosity > 1) {
        utstring_renew(outpath);
        utstring_printf(outpath, "%s/%s/bin",
                        utstring_body(opam_coswitch_lib),
                        pkg);

        log_info("Created %s containing symlinked pkg executables",
                 utstring_body(outpath));
    }


 stublibs: ;
    /* opam dumps all stublibs ('dllfoo.so') in lib/stublibs; they are
       not found in the pkg's lib subdir. But the package's
       dune-package file lists them, so we read that and then symlink
       them from lib/stublibs to lib/<pkg>/stublibs.
     */

    UT_array *stublibs = get_pkg_stublibs(s7, (char*)pkg, stanzas);
    if (utarray_len(stublibs) == 0) goto exit;

    UT_string *opam_stublib;
    utstring_new(opam_stublib);

    /* s7_pointer iter, binfile; */

    /* for most pkgs, WORKSPACE.bazel is already written,
       but for some containing only stublibs (e.g. menhir),
       we need to write it now
    */
    utstring_new(outpath);
    utstring_printf(outpath, "%s/%s/WORKSPACE.bazel",
                    utstring_body(opam_coswitch_lib),
                    pkg);

#if defined(TRACING)
    if (mibl_debug)
        log_debug("checking ws: %s", utstring_body(outpath));
#endif
    if (access(utstring_body(outpath), F_OK) != 0) {
#if defined(TRACING)
        log_debug("creating %s\n", utstring_body(outpath));
#endif
        /* if obazl/pkg not exist, create it with WORKSPACE */
        /* utstring_renew(outpath); */
        /* utstring_printf(outpath, "%s/%s", */
        /*                 utstring_body(opam_coswitch_lib), */
        /*                 pkg); */
        /* errno = 0; */
        /* if (access(utstring_body(outpath), F_OK) != 0) { */
        /*     utstring_printf(outpath, "/%s", "WORKSPACE.bazel"); */
        emit_workspace_file(outpath, pkg);
    }
    errno = 0;
    utstring_renew(outpath);
    utstring_printf(outpath, "%s/%s/stublibs",
                    utstring_body(opam_coswitch_lib),
                    //rootws, /* obazl, */
                    pkg);
    mkdir_r(utstring_body(outpath));

    utstring_renew(outpath);
    utstring_printf(outpath, "%s/%s/stublibs/BUILD.bazel",
                    utstring_body(opam_coswitch_lib),
                    pkg);
    /* rc = access(utstring_body(build_bazel_file), F_OK); */
#if defined(TRACING)
    log_debug("fopening: %s", utstring_body(outpath));
#endif

    /* FILE *ostream; */
    ostream = fopen(utstring_body(outpath), "w");
    if (ostream == NULL) {
        printf(RED "ERROR" CRESET "fopen failure for %s", utstring_body(outpath));
        log_error("fopen failure for %s", utstring_body(outpath));
        perror(utstring_body(outpath));
        log_error("Value of errno: %d", errnum);
        log_error("fopen error %s", strerror( errnum ));
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "## generated file - DO NOT EDIT\n");

    fprintf(ostream, "exports_files([");

    /* For each stublib, create symlink and exports_files entry */
    p = NULL;
    while ( (p=(char**)utarray_next(stublibs,p))) {
#if defined(TRACING)
        log_debug("stublib: %s",*p);
#endif
        fprintf(ostream, "\"%s\",", *p);
        /* symlink */
        utstring_renew(opam_stublib);
        utstring_printf(opam_stublib, "%s/stublibs/%s",
                        utstring_body(opam_switch_lib),
                        *p);
#if defined(TRACING)
        log_debug("SYMLINK SRC: %s", utstring_body(opam_stublib));
#endif
        utstring_renew(outpath);
        utstring_printf(outpath, "%s/%s/stublibs/%s",
                        utstring_body(opam_coswitch_lib),
                        pkg,
                        *p);
#if defined(TRACING)
        log_debug("SYMLINK DST: %s", utstring_body(outpath));
#endif
        int rc = symlink(utstring_body(opam_stublib),
                     utstring_body(outpath));
        symlink_ct++;
        if (rc != 0) {
            if (errno != EEXIST) {
                perror(NULL);
                fprintf(stderr, "ERROR,exiting\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    fprintf(ostream, "])\n");
    fclose(ostream);

    if (verbose && verbosity > 1) {
        utstring_renew(outpath);
        utstring_printf(outpath, "%s/%s/stublibs",
                        utstring_body(opam_coswitch_lib),
                        pkg);

        log_info("Created %s containing symlinked stublibs",
                 utstring_body(outpath));
    }

 exit: ;
    /* utstring_free(outpath); */
#if defined(TRACING)
    if (mibl_trace) printf("exiting emit_opam_pkg_bindir\n");
#endif
}

UT_string *dune_pkg_file;

/* pkg always relative to (global) opam_switch_lib */
EXPORT void emit_pkg_bindir(s7_scheme *s7, const char *pkg)
{
#if defined(TRACING)
    if (mibl_trace) log_trace("emit_pkg_bindir");
#endif

    utstring_renew(dune_pkg_file);
    utstring_printf(dune_pkg_file, "%s/%s/dune-package",
                    utstring_body(opam_switch_lib), /* global */
                    pkg);

#if defined(TRACING)
    if (mibl_trace)
        log_debug("CHECKING DUNE-PACKAGE: %s\n", utstring_body(dune_pkg_file));
#endif
    if (access(utstring_body(dune_pkg_file), F_OK) == 0) {
        emit_opam_pkg_bindir(s7, pkg); // dune_pkg_file);
                             /* switch_lib, */
                             /* pkgdir, */
                             /* obazl_opam_root, */
                             /* emitted_bootstrapper); */
    }
}
