#include <errno.h>
#include <dirent.h>
#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
/* #if EXPORT_INTERFACE */
#include <stdio.h>
/* #endif */
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "log.h"
#include "utarray.h"
#include "utstring.h"

/* #include "cjson/cJSON.h" */
/* #include "mustach-cjson.h" */
/* #include "mustach.h" */

#include "utils.h"
#include "findlib.h"

#include "emit_ocaml_repo.h"

extern int verbosity;

LOCAL const char *ws_name = "mibl";

/* extern UT_string *opam_switch_id; */
/* extern UT_string *opam_switch_prefix; */
extern UT_string *opam_ocaml_version;
extern UT_string *opam_switch_bin;
extern UT_string *opam_switch_lib;
extern UT_string *mibl_runfiles_root;

#if defined(TRACING)
bool mibl_debug_symlinks = false;
#endif

/* static char *readtemplate(const char *filename, size_t *length) */
/* { */
/*     errno = 0; */
/* #if defined(TRACING) */
/*         printf("cwd: %s\n", getcwd(NULL, 0)); */
/* #endif */

/*     UT_string *src; */
/*     utstring_new(src); */

/*     /\* FIXME: use a proper runfiles library *\/ */
/*     utstring_printf(src, */
/*                     "%s/external/%s/coswitch/templates/%s", */
/*                     utstring_body(mibl_runfiles_root), */
/*                     ws_name, */
/*                     filename); */
/*     int rc = access(utstring_body(src), F_OK); */
/*     if (rc != 0) { */
/*         perror(utstring_body(src)); */
/*         log_error("not found: %s", utstring_body(src)); */
/*         /\* fprintf(stderr, "not found: %s\n", utstring_body(src)); *\/ */
/*         exit(EXIT_FAILURE); */
/*     } */

/*     FILE *stream = fopen(utstring_body(src), "r"); */
/*     if (stream == NULL) { */
/*         printf(RED "ERROR: " CRESET "fopen %s: %s\n", */
/*                filename, strerror(errno)); */
/*         return NULL; */
/* #if defined(TRACING) */
/*     } else { */
/*         if (mibl_debug) printf("fopened %s\n", filename); */
/* #endif */
/*     } */

/*     fseek (stream, 0, SEEK_END); */
/*     long filea_sz = ftell (stream); */
/*     fseek (stream, 0, SEEK_SET); */
/*     char * buffer = malloc (filea_sz); */
/*     if (buffer == NULL) { */
/*         fprintf(stderr, "Out of memory reading template file %s\n", filename); */
/*         fclose(stream); */
/*         exit(EXIT_FAILURE); */
/*     } */

/*     size_t readed; */
/*     readed = fread(buffer, 1, (size_t)filea_sz, stream); */
/*     if (readed != filea_sz) { */
/*         fprintf(stderr, "fread readed ct != nitems\n"); */
/*         free(buffer); */
/*         fclose(stream); */
/*         exit(EXIT_FAILURE);     /\* FIXME: exit gracefully *\/ */
/*     } */
/*     fclose(stream); */
/*     utstring_free(src); */

/*     *length = readed; */

/* #if defined(TRACING) */
/*     log_debug("readed template: %ld (nitems: %ld)\n", readed, filea_sz); */
/* #endif */
/*     return buffer; */
/* } */

/* FIXME: put this in mustach lib */
/* static const char *mustach_errors[] = { */
/* 	"??? unreferenced ???", */
/* 	"system", */
/* 	"unexpected end", */
/* 	"empty tag", */
/* 	"tag too long", */
/* 	"bad separators", */
/* 	"too depth", */
/* 	"closing", */
/* 	"bad unescape tag", */
/* 	"invalid interface", */
/* 	"item not found", */
/* 	"partial not found", */
/* 	"undefined tag" */
/* }; */

/* void _process_mustache(char *template, UT_string *to_file) */
/* { */
/*     /\* log_debug("_process_mustache"); *\/ */
/*     /\* log_debug("opam_ocaml_version: %s", utstring_body(opam_ocaml_version)); *\/ */

/*     /\* static const char *errmsg = 0; *\/ */
/*     static int flags = Mustach_With_AllExtensions; */
/*     static FILE *output = 0; */

/*     errno = 0; */
/*     output = fopen(utstring_body(to_file), "w"); */
/*     if (output == NULL) { */
/*         fprintf(stdout, "%s", strerror(errno)); */
/*         exit(EXIT_FAILURE); */
/*     } */

/*     cJSON *version = cJSON_CreateString(utstring_body(opam_ocaml_version)); */
/*     cJSON *root    = cJSON_CreateObject(); */
/*     cJSON_AddItemToObject(root, "compiler_version", version); */

/*     size_t length; */
/*     char *template_str = readtemplate(template, &length); */

/*     /\* s = process(template_str, length); *\/ */
/*     int s = mustach_cJSON_file(template_str, length, root, flags, output); */

/*     free(template_str); */
/*     if (s != MUSTACH_OK) { */
/*         s = -s; */
/*         if (s < 1 || s >= (int)(sizeof mustach_errors / sizeof * mustach_errors)) */
/*             s = 0; */
/*         fprintf(stderr, "Template error %s (file %s)\n", mustach_errors[s], template); */
/*     } */
/*     fclose(output); */
/*     cJSON_Delete(root); */
/* } */

void _copy_buildfile(char *buildfile, UT_string *to_file)
{
    UT_string *src;
    utstring_new(src);

    /* FIXME: use a proper runfiles library */
    utstring_printf(src,
                    "%s/external/%s/coswitch/templates/%s",
                    utstring_body(mibl_runfiles_root),
                    ws_name,
                    buildfile);
    int rc = access(utstring_body(src), F_OK);
    if (rc != 0) {
        perror(utstring_body(src));
        log_error("not found: %s", utstring_body(src));
        /* fprintf(stderr, "not found: %s\n", utstring_body(src)); */
        exit(EXIT_FAILURE);
        return;
    }

    /* if (mibl_debug) { */
    /*     log_debug("copying %s to %s\n", */
    /*               utstring_body(src), */
    /*               utstring_body(to_file)); */
    /* } */
    errno = 0;
    rc = copyfile(utstring_body(src),
                  utstring_body(to_file));
    if (rc != 0) {
        log_error("copyfile: %s", strerror(errno));
        fprintf(stderr, "ERROR copyfile: %s", strerror(errno));
        log_error("Exiting");
        fprintf(stderr, "Exiting\n");
        exit(EXIT_FAILURE);
    }
}

FILE *_open_buildfile(UT_string *ocaml_file) {
    FILE *ostream = fopen(utstring_body(ocaml_file), "w");
    ostream = fopen(utstring_body(ocaml_file), "w");
    if (ostream == NULL) {
        log_error("fopen: %s: %s", strerror(errno),
                  utstring_body(ocaml_file));
        fprintf(stderr, "fopen: %s: %s", strerror(errno),
                utstring_body(ocaml_file));
        fprintf(stderr, "exiting\n");
        /* perror(utstring_body(ocaml_file)); */
        exit(EXIT_FAILURE);
    }
    return ostream;
}

/* void emit_ocaml_stdlib_pkg(char *switch_name) */
/* { */
/*     if (mibl_debug) */
/*         log_debug("emit_ocaml_stdlib_pkg"); */

/*     UT_string *ocaml_file; */
/*     utstring_new(ocaml_file); */
/*     utstring_concat(ocaml_file, bzl_switch_pfx); */
/*     utstring_printf(ocaml_file, "/ocaml/lib/stdlib"); */
/*     mkdir_r(utstring_body(ocaml_file)); */

/*     _symlink_ocaml_stdlib(utstring_body(ocaml_file)); */

/*     utstring_printf(ocaml_file, "/BUILD.bazel"); */

/*     _copy_buildfile("ocaml_stdlib.BUILD", ocaml_file); */
/*     utstring_free(ocaml_file); */
/* } */

/* void _symlink_ocaml_stdlib(char *tgtdir) */
/* { */
/*     if (mibl_debug) */
/*         log_debug("_symlink_ocaml_stdlib to %s\n", tgtdir); */

/*     UT_string *opamdir; */
/*     utstring_new(opamdir); */
/*     utstring_printf(opamdir, "%s/ocaml", utstring_body(opam_switch_lib)); */

/*     UT_string *src; */
/*     utstring_new(src); */
/*     UT_string *dst; */
/*     utstring_new(dst); */
/*     int rc; */

/*     DIR *d = opendir(utstring_body(opamdir)); */
/*     if (d == NULL) { */
/*         fprintf(stderr, "Unable to opendir for symlinking stdlib: %s\n", */
/*                 utstring_body(opamdir)); */
/*         /\* exit(EXIT_FAILURE); *\/ */
/*         return; */
/*     } */

/*     struct dirent *direntry; */
/*     while ((direntry = readdir(d)) != NULL) { */
/*         if(direntry->d_type==DT_REG){ */
/*             if (strncmp("stdlib", direntry->d_name, 6) != 0) */
/*                 continue; */

/*             utstring_renew(src); */
/*             utstring_printf(src, "%s/%s", */
/*                             utstring_body(opamdir), direntry->d_name); */
/*             utstring_renew(dst); */
/*             utstring_printf(dst, "%s/%s", */
/*                             tgtdir, direntry->d_name); */
/*             /\* printf("symlinking %s to %s\n", *\/ */
/*             /\*        utstring_body(src), *\/ */
/*             /\*        utstring_body(dst)); *\/ */
/*             rc = symlink(utstring_body(src), */
/*                          utstring_body(dst)); */
/*             if (rc != 0) { */
/*                 if (errno != EEXIST) { */
/*                     perror(utstring_body(src)); */
/*                     fprintf(stderr, "exiting\n"); */
/*                     exit(EXIT_FAILURE); */
/*                 } */
/*             } */
/*         } */
/*     } */
/*     closedir(d); */
/* } */

void emit_ocaml_runtime_pkg(char *bzl_switch_lib)  // dest
{
#if defined(TRACING)
    if (mibl_trace) log_trace("emit_ocaml_runtime_pkg");
#endif

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    /* utstring_printf(ocaml_file, bzl_switch_lib); // pfx); */
    utstring_printf(ocaml_file, "%s/ocaml/runtime", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));

    /* _symlink_ocaml_runtime(utstring_body(ocaml_file)); */
    _symlink_ocaml_runtime(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_runtime.BUILD", ocaml_file);
    utstring_free(ocaml_file);
}

void _symlink_ocaml_runtime(char *tgtdir)
{
#if defined(TRACING)
    if (mibl_debug_symlinks)
        log_trace("_symlink_ocaml_runtime to %s", tgtdir);
#endif

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml", utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        perror(utstring_body(opamdir));
        log_error(RED "Unable to opendir for symlinking stdlib" CRESET);
        exit(EXIT_FAILURE);
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            if (strncmp("stdlib", direntry->d_name, 6) != 0)
                if (strncmp("std_exit", direntry->d_name, 8) != 0)
                    continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    perror(utstring_body(src));
                    fprintf(stderr, "exiting\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
}

/* **************************************************************** */
void emit_ocaml_stublibs(char *bzl_switch_lib)
{
#if defined(TRACING)
    if (mibl_trace)
        log_trace("emit_ocaml_stublibs: %s/ocaml/stublibs", bzl_switch_lib);
#endif
    UT_string *ocaml_file;

    /* **************** */
    utstring_new(ocaml_file);
    /* utstring_printf(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/ocaml/stublibs", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");

    FILE *ostream = fopen(utstring_body(ocaml_file), "w");
    ostream = fopen(utstring_body(ocaml_file), "w");
    if (ostream == NULL) {
        perror(utstring_body(ocaml_file));
        log_error("fopen: %s: %s", strerror(errno),
                  utstring_body(ocaml_file));
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "# generated file - DO NOT EDIT\n");
    /* fprintf(ostream, "exports_files(glob([\"**\"]))\n"); */
    fprintf(ostream, "filegroup(\n");
    fprintf(ostream, "    name = \"stublibs\",\n");
    fprintf(ostream, "    srcs = glob([\"**\"]),\n");
    fprintf(ostream, ")\n");
    fclose(ostream);

    /* _emit_ocaml_stublibs_symlinks("/ocaml/stublibs"); */
}

void _emit_ocaml_stublibs_symlinks(char *_dir)
{
#if defined(TRACING)
    if (mibl_trace) log_trace("_emit_ocaml_stublibs_symlinks: %s", _dir);
#endif
     // _dir ==? "/ocaml/stublibs");
    UT_string *dst_dir;
    utstring_new(dst_dir);
    /* utstring_printf(dst_dir, bzl_switch_pfx); */
    utstring_printf(dst_dir, "%s/%s", utstring_body(bzl_switch_pfx), _dir);
    mkdir_r(utstring_body(dst_dir));

    UT_string *src_dir; // relative to opam_switch_lib
    utstring_new(src_dir);
    utstring_printf(src_dir,
                    "%s%s",
                    /* "%s/ocaml/stublibs", */
                    utstring_body(opam_switch_lib),
                    _dir);

#if defined(TRACING)
    log_debug("src_dir: %s\n", utstring_body(src_dir));
    log_debug("dst_dir: %s\n", utstring_body(dst_dir));
#endif

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

#if defined(TRACING)
    log_debug("opening src_dir for read: %s\n",
              utstring_body(src_dir));
#endif
    DIR *srcd = opendir(utstring_body(src_dir));
    /* DIR *srcd = opendir(utstring_body(opamdir)); */
    if (srcd == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking stublibs: %s\n",
                utstring_body(src_dir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(srcd)) != NULL) {
        //Condition to check regular file.
#if defined(TRACING)
        if (mibl_debug)
            log_debug("stublib: %s, type %d",
                      direntry->d_name, direntry->d_type);
#endif
        if( (direntry->d_type==DT_REG)
            || (direntry->d_type==DT_LNK) ){

            /* do not symlink workspace file */
            if (strncmp(direntry->d_name, "WORKSPACE", 9) == 0) {
                continue;
            }
            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(src_dir),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            utstring_body(dst_dir), direntry->d_name);

#if defined(TRACING)
            if (mibl_debug) {
                log_debug("stublibs: symlinking %s to %s\n",
                          utstring_body(src),
                          utstring_body(dst));
            }
#endif

            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                switch (errno) {
                case EEXIST:
                    goto ignore;
                case ENOENT:
                    log_error("symlink ENOENT: %s", strerror(errno));
                    log_error("a component of '%s' does not name an existing file",  utstring_body(dst_dir));
                    fprintf(stderr, "symlink ENOENT: %s\n", strerror(errno));
                    fprintf(stderr, "A component of '%s' does not name an existing file.\n",  utstring_body(dst_dir));
                    break;
                default:
                    log_error("symlink err: %s", strerror(errno));
                    fprintf(stderr, "symlink err: %s", strerror(errno));
                }
                log_error("Exiting");
                fprintf(stderr, "Error, exiting\n");
                exit(EXIT_FAILURE);
            ignore:
                ;
            }
        }
    }
    closedir(srcd);
}

/* **************************************************************** */
void emit_lib_stublibs(char *bzl_switch_lib)
{
#if defined(TRACING)
    if (mibl_trace) log_trace("emit_lib_stublibs");
#endif
    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/stublibs", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/WORKSPACE.bazel");

    FILE *ostream = fopen(utstring_body(ocaml_file), "w");
    ostream = fopen(utstring_body(ocaml_file), "w");
    if (ostream == NULL) {
        log_error("fopen: %s: %s", strerror(errno),
                  utstring_body(ocaml_file));
        fprintf(stderr, "fopen: %s: %s", strerror(errno),
                utstring_body(ocaml_file));
        fprintf(stderr, "exiting\n");
        /* perror(utstring_body(ocaml_file)); */
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "workspace( name = \"stublibs\" )"
            "    # generated file - DO NOT EDIT\n");
    fclose(ostream);

    /* now BUILD.bazel */
    utstring_renew(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/stublibs/lib/stublibs",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");

    ostream = fopen(utstring_body(ocaml_file), "w");
    if (ostream == NULL) {
        log_error("fopen: %s: %s", strerror(errno),
                  utstring_body(ocaml_file));
        fprintf(stderr, "fopen: %s: %s", strerror(errno),
                utstring_body(ocaml_file));
        fprintf(stderr, "exiting\n");
        /* perror(utstring_body(ocaml_file)); */
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "# generated file - DO NOT EDIT\n\n");
    /* fprintf(ostream, "exports_files(glob([\"**\"]))\n"); */
    fprintf(ostream, "filegroup(\n");
    fprintf(ostream, "    name = \"stublibs\",\n");
    fprintf(ostream, "    srcs = glob([\"**\"]),\n");
    fprintf(ostream, "    visibility = [\"//visibility:public\"]\n");
    fprintf(ostream, ")\n");
    fclose(ostream);
    utstring_free(ocaml_file);
    /* **************************************************************** */
    _emit_lib_stublibs_symlinks(bzl_switch_lib);
}

void _emit_lib_stublibs_symlinks(char *bzl_switch_lib)
{
#if defined(TRACING)
    if (mibl_debug_symlinks)
        log_trace("_emit_lib_stublibs_symlinks: %s", bzl_switch_lib);
#endif

    UT_string *dst_dir;
    utstring_new(dst_dir);
    /* utstring_printf(dst_dir, bzl_switch_pfx); */
    utstring_printf(dst_dir, "%s/stublibs/lib/stublibs",
                    bzl_switch_lib);
    mkdir_r(utstring_body(dst_dir));

    UT_string *src_dir; // relative to opam_switch_lib
    utstring_new(src_dir);
    utstring_printf(src_dir,
                    "%s%s",
                    utstring_body(opam_switch_lib),
                    "/stublibs"); // _dir);

    /* log_debug("src_dir: %s\n", utstring_body(src_dir)); */
    /* log_debug("dst_dir: %s\n", utstring_body(dst_dir)); */

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

/* #if defined(TRACING) */
/*     log_debug("opening src_dir for read: %s", */
/*               utstring_body(src_dir)); */
/* #endif */
    DIR *srcd = opendir(utstring_body(src_dir));
    /* DIR *srcd = opendir(utstring_body(opamdir)); */
    if (srcd == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking stublibs: %s\n",
                utstring_body(src_dir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(srcd)) != NULL) {
        //Condition to check regular file.
#if defined(TRACING)
        if (mibl_debug_symlinks) {
            log_debug("stublib: %s, type %d",
                      direntry->d_name, direntry->d_type);
        }
#endif
        if (strncmp(direntry->d_name, "WORKSPACE", 9) == 0) {
            continue;
        }

        if( (direntry->d_type==DT_REG)
            || (direntry->d_type==DT_LNK) ){

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(src_dir),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            utstring_body(dst_dir), direntry->d_name);

#if defined(TRACING)
            if (mibl_debug_symlinks) {
                log_debug("stublibs: symlinking %s to %s",
                          utstring_body(src),
                          utstring_body(dst));
            }
#endif

            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                switch (errno) {
                case EEXIST:
                    goto ignore;
                case ENOENT:
                    log_error("symlink ENOENT: %s", strerror(errno));
                    log_error("a component of '%s' does not name an existing file",  utstring_body(dst_dir));
                    fprintf(stderr, "symlink ENOENT: %s\n", strerror(errno));
                    fprintf(stderr, "A component of '%s' does not name an existing file.\n",  utstring_body(dst_dir));
                    break;
                default:
                    log_error("symlink err: %s", strerror(errno));
                    fprintf(stderr, "symlink err: %s", strerror(errno));
                }
                log_error("Exiting");
                fprintf(stderr, "Exiting\n");
                exit(EXIT_FAILURE);
            ignore:
                ;
            }
        }
    }
    closedir(srcd);
}

/* **************************************************************** */
void emit_ocaml_platform_buildfiles(char *bzl_switch_lib)
{
#if defined(TRACING)
    if (mibl_trace)
        log_trace("emit_ocaml_platform_buildfiles: %s", bzl_switch_lib);
#endif
    UT_string *ocaml_file;

    /* platform definitions */
    utstring_new(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */

    utstring_printf(ocaml_file, "%s/ocaml/platforms", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("platforms/BUILD.bazel", ocaml_file);

    utstring_renew(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/platforms/build", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("platforms/build/BUILD.bazel", ocaml_file);

    utstring_renew(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/platforms/target", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("platforms/target/BUILD.bazel", ocaml_file);

    utstring_free(ocaml_file);
}

void emit_ocaml_toolchain_buildfiles(char *bzl_switch_lib)
{
#if defined(TRACING)
    if (mibl_trace)
        log_trace("emit_ocaml_toolchain_buildfiles: %s", bzl_switch_lib);
#endif
    UT_string *ocaml_file;

    /* platform definitions */
    utstring_new(ocaml_file);
    /* utstring_new(ocaml_file); */
    utstring_printf(ocaml_file, "%s/ocaml/toolchain/selectors/local",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/selectors/local.BUILD", ocaml_file);

    utstring_renew(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/toolchain/selectors/macos/x86_64",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/selectors/macos/x86_64.BUILD", ocaml_file);

    utstring_new(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/toolchain/selectors/macos/arm",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/selectors/macos/arm.BUILD", ocaml_file);

    utstring_new(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/toolchain/selectors/linux/x86_64",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/selectors/linux/x86_64.BUILD", ocaml_file);

    utstring_new(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/toolchain/selectors/linux/arm",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/selectors/linux/arm.BUILD", ocaml_file);

    /* toolchain options */
    utstring_new(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/toolchain/profiles",
                    bzl_switch_lib);

    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/profiles/profiles.BUILD", ocaml_file);

    /* toolchain adapters */
    utstring_new(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/toolchain/adapters/local",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/adapters/local.BUILD", ocaml_file);
    //TODO: mustache support
    /* _process_mustache("toolchain/adapters/local.BUILD.mustache", ocaml_file); */


    utstring_new(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/toolchain/adapters/linux/x86_64",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/adapters/linux/x86_64.BUILD", ocaml_file);

    utstring_new(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/toolchain/adapters/linux/arm",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/adapters/linux/arm.BUILD", ocaml_file);

    utstring_new(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/toolchain/adapters/macos/x86_64",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/adapters/macos/x86_64.BUILD", ocaml_file);

    utstring_new(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/toolchain/adapters/macos/arm",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/adapters/macos/arm.BUILD", ocaml_file);

    utstring_free(ocaml_file);
}

/* void emit_ocaml_toolchain_buildfile(char *bzl_switch_lib) */
/* { */
/* #if defined(TRACING) */
/*     if (mibl_trace) log_trace("emit_ocaml_toolchain_buildfile"); */
/* #endif */
/*     UT_string *ocaml_file; */
/*     utstring_new(ocaml_file); */
/*     /\* utstring_concat(ocaml_file, bzl_switch_pfx); *\/ */
/*     utstring_printf(ocaml_file, "%s/ocaml/toolchains", */
/*                     bzl_switch_lib); */
/*     mkdir_r(utstring_body(ocaml_file)); */
/*     utstring_printf(ocaml_file, "/BUILD.bazel"); */
/*     _copy_buildfile("ocaml_toolchains.BUILD", ocaml_file); */
/*     utstring_free(ocaml_file); */
/* } */

void emit_ocaml_bin_dir(char *bzl_switch_lib)  // dest
{
#if defined(TRACING)
    if (mibl_trace) log_trace("emit_ocaml_bin_dir");
#endif
    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/bin", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");

    FILE *ostream;
    ostream = fopen(utstring_body(ocaml_file), "w");
    if (ostream == NULL) {
        log_error("fopen: %s: %s", strerror(errno),
                  utstring_body(ocaml_file));
        fprintf(stderr, "fopen: %s: %s", strerror(errno),
                utstring_body(ocaml_file));
        fprintf(stderr, "exiting\n");
        /* perror(utstring_body(ocaml_file)); */
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "# generated file - DO NOT EDIT\n");
    fprintf(ostream, "exports_files(glob([\"**\"]))\n");
    fclose(ostream);
    utstring_free(ocaml_file);

    /* **************************************************************** */
    _emit_ocaml_bin_symlinks(bzl_switch_lib);
}

void _emit_ocaml_bin_symlinks(char *bzl_switch_lib)  // dest
{
#if defined(TRACING)
    if (mibl_debug_symlinks)
        log_debug("_emit_ocaml_bin_symlinks");
#endif

    UT_string *dst_dir;
    utstring_new(dst_dir);
    /* utstring_concat(dst_dir, bzl_switch_lib); // pfx); */
    utstring_printf(dst_dir, "%s/ocaml/bin", bzl_switch_lib);

    /* UT_string *src_dir; // relative to opam_switch_lib */
    /* utstring_new(src_dir); */
    /* utstring_printf(src_dir, "bin"); */

    mkdir_r(utstring_body(dst_dir));

#if defined(TRACING)
    if (mibl_debug) {
        log_debug("src_dir: %s\n", opam_switch_bin);
        log_debug("dst_dir: %s\n", utstring_body(dst_dir));
    }
#endif

    /* UT_string *opamdir; */
    /* utstring_new(opamdir); */
    /* utstring_printf(opamdir, "%s/bin", */
    /*                 utstring_body(opam_switch_pfx) */
    /*                 /\* utstring_body(src_dir) *\/ */
    /*                 ); */

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

#if defined(TRACING)
    if (mibl_debug)
        log_debug("opening src_dir for read: %s\n",
                  utstring_body(opam_switch_bin));
#endif
    DIR *srcd = opendir(utstring_body(opam_switch_bin));
    /* DIR *srcd = opendir(utstring_body(opamdir)); */
    if (dst == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking toolchain: %s\n",
                utstring_body(opam_switch_bin));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(srcd)) != NULL) {
        //Condition to check regular file.
        if( (direntry->d_type==DT_REG)
            || (direntry->d_type==DT_LNK) ){

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opam_switch_bin),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            utstring_body(dst_dir), direntry->d_name);

            /* log_debug("symlinking %s to %s", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */

            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                switch (errno) {
                case EEXIST:
                    goto ignore;
                case ENOENT:
                    log_error("symlink ENOENT: %s", strerror(errno));
                    log_error("a component of '%s' does not name an existing file",  utstring_body(dst_dir));
                    fprintf(stderr, "symlink ENOENT: %s\n", strerror(errno));
                    fprintf(stderr, "A component of '%s' does not name an existing file.\n",  utstring_body(dst_dir));
                    break;
                default:
                    log_error("symlink err: %s", strerror(errno));
                    fprintf(stderr, "symlink err: %s", strerror(errno));
                }
                log_error("Exiting");
                fprintf(stderr, "Exiting\n");
                exit(EXIT_FAILURE);
            ignore:
                ;
            }
        }
    }
    closedir(srcd);
}

/* **************************************************************** */
/* obsolete but keep it around in case we decide to use it later */
void _symlink_buildfile(char *buildfile, UT_string *to_file)
{
#if defined(TRACING)
    if (mibl_debug_symlinks) log_trace("_symlink_buildfile");
#endif
    UT_string *src;
    utstring_new(src);
    utstring_printf(src,
                    "%s/external/%s/coswitch/templates/%s",
                    utstring_body(mibl_runfiles_root),
                    ws_name,
                    buildfile);
    int rc = access(utstring_body(src), F_OK);
    if (rc != 0) {
        log_error("not found: %s", utstring_body(src));
        fprintf(stderr, "not found: %s\n", utstring_body(src));
        return;
    }

#if defined(TRACING)
    if (mibl_debug) {
        log_debug("c_libs: symlinking %s to %s\n",
                  utstring_body(src),
                  utstring_body(to_file));
    }
#endif
    errno = 0;
    rc = symlink(utstring_body(src),
                 utstring_body(to_file));
    symlink_ct++;
    if (rc != 0) {
        switch (errno) {
        case EEXIST:
            goto ignore;
        case ENOENT:
            log_error("symlink ENOENT: %s", strerror(errno));
            /* log_error("a component of '%s' does not name an existing file",  utstring_body(dst_dir)); */
            fprintf(stderr, "symlink ENOENT: %s\n", strerror(errno));
            /* fprintf(stderr, "A component of '%s' does not name an existing file.\n",  utstring_body(dst_dir)); */
            break;
        default:
            log_error("symlink err: %s", strerror(errno));
            fprintf(stderr, "symlink err: %s", strerror(errno));
        }
        log_error("Exiting");
        fprintf(stderr, "Exiting\n");
        exit(EXIT_FAILURE);
    ignore:
        ;
    }
}

/* **************************************************************** */
void emit_ocaml_bigarray_pkg(char *bzl_switch_lib)
{
#if defined(TRACING)
    if (mibl_trace) log_debug("emit_ocaml_bigarray_pkg");
#endif

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/ocaml/bigarray", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));

    _symlink_ocaml_bigarray(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_bigarray.BUILD", ocaml_file);
    utstring_free(ocaml_file);
}

void _symlink_ocaml_bigarray(char *tgtdir)
{
#if defined(TRACING)
    if (mibl_debug_symlinks)
        log_debug("_symlink_ocaml_bigarray to %s", tgtdir);
#endif

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml", utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        perror(utstring_body(opamdir));
        log_error(RED "Unable to opendir for symlinking bigarray" CRESET " %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strcasestr(direntry->d_name, "bigarray") == NULL) */
            if (strncmp("bigarray", direntry->d_name, 8) != 0)
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    perror(utstring_body(src));
                    fprintf(stderr, "exiting\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
}

/* **************************************************************** */
/* for <switch>/lib/ocaml/compiler-libs */
void emit_ocaml_compiler_libs_pkg(char *bzl_switch_lib)
{
#if defined(TRACING)
    if (mibl_trace) log_debug("emit_ocaml_compiler_libs_pkg");
#endif

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/ocaml/compiler-libs", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));

    /* _symlink_ocaml_compiler_libs(utstring_body(ocaml_file)); */

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_compiler-libs.BUILD", ocaml_file);
    /* _symlink_buildfile("ocaml_compiler-libs.BUILD", ocaml_file); */

    utstring_renew(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/ocaml/compiler-libs/common",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("compiler_libs/common.BUILD", ocaml_file);

    utstring_renew(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/ocaml/compiler-libs/bytecomp",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("compiler_libs/bytecomp.BUILD", ocaml_file);

    utstring_renew(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/ocaml/compiler-libs/optcomp",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("compiler_libs/optcomp.BUILD", ocaml_file);

    utstring_renew(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/ocaml/compiler-libs/toplevel",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("compiler_libs/toplevel.BUILD", ocaml_file);

    utstring_free(ocaml_file);
}

/* for <switch>/lib/compiler-libs */
void emit_compiler_libs_pkg(char *bzl_switch_lib)
{
#if defined(TRACING)
    if (mibl_trace) log_debug("emit_compiler_libs_pkg");
#endif

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/compiler-libs/lib/compiler-libs", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));

    /* _symlink_ocaml_compiler_libs(utstring_body(ocaml_file)); */

    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("compiler_libs/common.BUILD", ocaml_file);

    utstring_renew(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/compiler-libs/lib/common",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("compiler_libs/common.BUILD", ocaml_file);

    utstring_renew(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/compiler-libs/lib/bytecomp",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("compiler_libs/bytecomp.BUILD", ocaml_file);

    utstring_renew(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/compiler-libs/lib/optcomp",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("compiler_libs/optcomp.BUILD", ocaml_file);

    utstring_renew(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/compiler-libs/lib/toplevel",
                    bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("compiler_libs/toplevel.BUILD", ocaml_file);

    utstring_free(ocaml_file);
}

void _symlink_ocaml_compiler_libs(char *tgtdir)
{
#if defined(TRACING)
    if (mibl_debug_symlinks)
        log_debug("_symlink_ocaml_compiler_libs to %s\n", tgtdir);
#endif

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml/compiler-libs",
                    utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking compiler-libs: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);

            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            log_debug("symlinking %s to %s\n",
                      utstring_body(src),
                      utstring_body(dst));
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    perror(utstring_body(src));
                    fprintf(stderr, "exiting\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
}

/* ***************************************** */
void emit_ocaml_dynlink_pkg(char *bzl_switch_lib)
{
#if defined(TRACING)
    if (mibl_trace) log_trace("emit_ocaml_dynlink_pkg");
#endif

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/ocaml/dynlink", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));

    _symlink_ocaml_dynlink(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_dynlink.BUILD", ocaml_file);
    /* _symlink_buildfile("ocaml_dynlinks.BUILD", ocaml_file); */

    utstring_free(ocaml_file);
}

void _symlink_ocaml_dynlink(char *tgtdir)
{
#if defined(TRACING)
    if (mibl_debug_symlinks)
        log_debug("_symlink_ocaml_dynlink to %s", tgtdir);
#endif

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml", utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking dynlink: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strcasestr(direntry->d_name, "dynlink") == NULL) */
            if (strncmp("dynlink", direntry->d_name, 7) != 0)
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    perror(utstring_body(src));
                    fprintf(stderr, "exiting\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
}

/* ***************************************** */
void emit_ocaml_num_pkg(char *bzl_switch_lib)
{ /* only if 'nums' pseudo-pkg was installed */
#if defined(TRACING)
    if (mibl_trace) log_debug("emit_ocaml_num_pkg");
#endif

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_printf(ocaml_file, "%s/num/META",
                    utstring_body(opam_switch_lib));
    int rc = access(utstring_body(ocaml_file), F_OK);
    if (rc != 0) {
        /* pkg 'num' not installed */
#if defined(TRACING)
        if (mibl_trace) log_trace(YEL "num pkg not installed" CRESET);
#endif
        return;
    }

    utstring_renew(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/ocaml/num/core", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));

    /* log_debug("ocaml_file: %s", utstring_body(ocaml_file)); */
    _symlink_ocaml_num(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_num.BUILD", ocaml_file);

    utstring_free(ocaml_file);
}

void _symlink_ocaml_num(char *tgtdir)
{
#if defined(TRACING)
    if (mibl_debug_symlinks)
        log_debug("_symlink_ocaml_num to %s", tgtdir);
#endif

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml", utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        perror(utstring_body(opamdir));
        log_error("Unable to opendir for symlinking num: %s\n",
                  utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }
#if defined(TRACING)
    if (mibl_debug)
        log_debug("reading num dir %s", utstring_body(opamdir));
#endif

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strcasestr(direntry->d_name, "num") == NULL) */
            if (strncmp("nums.", direntry->d_name, 5) != 0)
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
#if defined(TRACING)
            if (mibl_debug_symlinks)
                printf("symlinking %s to %s\n",
                       utstring_body(src),
                       utstring_body(dst));
#endif
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    perror(utstring_body(src));
                    fprintf(stderr, "exiting\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
}

/* ************************************* */
void emit_ocaml_str_pkg(char *bzl_switch_lib)
{
#if defined(TRACING)
    if (mibl_trace) log_debug("emit_ocaml_str_pkg");
#endif

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/ocaml/str", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));

    _symlink_ocaml_str(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_str.BUILD", ocaml_file);
    utstring_free(ocaml_file);
}

void _symlink_ocaml_str(char *tgtdir)
{
#if defined(TRACING)
    if (mibl_debug)
        log_debug("_symlink_ocaml_str to %s", tgtdir);
#endif

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml", utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking str: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strcasestr(direntry->d_name, "str.") == NULL) */
            if (strncmp("str.", direntry->d_name, 4) != 0)
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    perror(utstring_body(src));
                    fprintf(stderr, "exiting\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
}

/* **************************************************************** */
void emit_ocaml_threads_pkg(char *bzl_switch_lib)
{
#if defined(TRACING)
    if (mibl_trace) log_trace("emit_ocaml_threads_pkg");
#endif

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/threads", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));

    /* _symlink_ocaml_threads(utstring_body(ocaml_file)); */

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_threads.BUILD", ocaml_file);
    utstring_free(ocaml_file);
}

void _symlink_ocaml_threads(char *tgtdir)
{
#if defined(TRACING)
    if (mibl_debug_symlinks)
        log_debug("_symlink_ocaml_threads to %s\n", tgtdir);
#endif

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml/threads",
                    utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking threads: %s\n",
                utstring_body(opamdir));
        log_error("Unable to opendir for symlinking threads: %s\n", utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    //FIXME: just open ocaml/threads directly instead of searching for it

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strcasestr(direntry->d_name, "threads") == NULL) */
            /*     continue; */

            /* log_debug("threads dirent: %s", direntry->d_name); */

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    perror(utstring_body(src));
                    fprintf(stderr, "exiting\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
}

/* ***************************************** */
void emit_ocaml_ocamldoc_pkg(char *bzl_switch_lib)
{
#if defined(TRACING)
    if (mibl_debug) log_debug("emit_ocaml_ocamldoc_pkg");
#endif

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_pfx); */
    utstring_printf(ocaml_file, "%s/ocaml/ocamldoc", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));

    /* _symlink_ocaml_ocamldoc(utstring_body(ocaml_file)); */

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_ocamldoc.BUILD", ocaml_file);

    utstring_free(ocaml_file);
}

void _symlink_ocaml_ocamldoc(char *tgtdir)
{
#if defined(TRACING)
    if (mibl_debug_symlinks)
        log_debug("_symlink_ocaml_ocamldoc to %s\n", tgtdir);
#endif

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml/ocamldoc",
                    utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking ocamldoc: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strcasestr(direntry->d_name, "ocamldoc") == NULL) */
            /* if (strncmp("odoc", direntry->d_name, 4) != 0) */
            /*     continue; */

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    perror(utstring_body(src));
                    fprintf(stderr, "exiting\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
}

/* **************************************************************** */
void emit_ocaml_unix_pkg(char *bzl_switch_lib)
{
#if defined(TRACING)
    if (mibl_trace) log_trace("emit_ocaml_unix_pkg");
#endif

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/unix", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));

    _symlink_ocaml_unix(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_unix.BUILD", ocaml_file);
    utstring_free(ocaml_file);
}

void _symlink_ocaml_unix(char *tgtdir)
{
#if defined(TRACING)
    if (mibl_debug_symlinks)
        log_debug("_symlink_ocaml_unix to %s\n", tgtdir);
#endif

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml", utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking unix: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* printf("dirent: %s/%s\n", */
            /*        utstring_body(opamdir), direntry->d_name); */

            /* if (strncmp(direntry->d_name, "unix", 4) != 0) */
            /*      continue; */

            if (strncmp("unix", direntry->d_name, 4) != 0)
            /* if (strcasestr(direntry->d_name, "unix") == NULL) */
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    perror(utstring_body(src));
                    fprintf(stderr, "exiting\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
}

/* **************************************************************** */
void emit_ocaml_c_api_pkg(char *bzl_switch_lib)
{
#if defined(TRACING)
    if (mibl_trace) log_trace("emit_ocaml_c_api_pkg");
#endif

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_printf(ocaml_file, "%s/ocaml/c", bzl_switch_lib);
    _symlink_ocaml_c_hdrs(utstring_body(ocaml_file));
    _symlink_ocaml_c_libs(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_c_api.BUILD", ocaml_file);
    /* _symlink_buildfile("ocaml_c_api.BUILD", ocaml_file); */

    utstring_free(ocaml_file);
}

void _symlink_ocaml_c_hdrs(char *tgtdir)
{
#if defined(TRACING)
    if (mibl_debug_symlinks)
        log_debug("_symlink_ocaml_c_hdrs to %s\n", tgtdir);
#endif

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml/caml",
                    utstring_body(opam_switch_lib));

    UT_string *obazldir;
    utstring_new(obazldir);
    utstring_printf(obazldir, "%s/caml", tgtdir);
    mkdir_r(utstring_body(obazldir));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking str: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strncmp("lib", direntry->d_name, 3) != 0) */
            /*     continue;       /\* no match *\/ */
            //FIXME: check for .h?

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            utstring_body(obazldir), /* tgtdir, */
                            direntry->d_name);
#if defined(TRACING)
            /* if (mibl_debug_symlinks) { */
            /*     log_debug("c_hdrs: symlinking %s to %s\n", */
            /*               utstring_body(src), */
            /*               utstring_body(dst)); */
            /* } */
#endif
            errno = 0;
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    fprintf(stdout, "c_hdrs: symlink errno: %d: %s\n",
                            errno, strerror(errno));
                    fprintf(stdout, "c_hdrs src: %s, dst: %s\n",
                            utstring_body(src),
                            utstring_body(dst));
                    fprintf(stderr, "exiting\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
}

void _symlink_ocaml_c_libs(char *tgtdir)
{
#if defined(TRACING)
    if (mibl_debug_symlinks)
        log_debug("_symlink_ocaml_c_libs to %s\n", tgtdir);
#endif

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml",
                    utstring_body(opam_switch_lib));

    UT_string *obazldir;
    utstring_new(obazldir);
    utstring_printf(obazldir, "%s/lib", tgtdir);
    mkdir_r(utstring_body(obazldir));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking c_libs: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            if (strncmp("lib", direntry->d_name, 3) != 0)
                continue;       /* no match */

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            utstring_body(obazldir), /* tgtdir, */
                            direntry->d_name);
#if defined(TRACING)
            /* if (mibl_debug_symlinks) { */
            /*     log_debug("c_libs: symlinking %s to %s\n", */
            /*               utstring_body(src), */
            /*               utstring_body(dst)); */
            /* } */
#endif
            errno = 0;
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    fprintf(stdout, "c_libs symlink errno: %d: %s\n",
                            errno, strerror(errno));
                    fprintf(stdout, "c_libs src: %s, dst: %s\n",
                            utstring_body(src),
                            utstring_body(dst));
                    fprintf(stderr, "exiting\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
}

/* **************************************************************** */
void emit_ocaml_bootstrap(char *opam_switch, FILE *bootstrap_FILE)
{
    (void)opam_switch;
#if defined(TRACING)
    if (mibl_trace) log_trace("emit_ocaml_bootstrap");
#endif

    fprintf(bootstrap_FILE, "    native.local_repository(\n");
    fprintf(bootstrap_FILE, "        name       = \"ocaml\",\n");

    fprintf(bootstrap_FILE, "        path       = ");
    fprintf(bootstrap_FILE, "\"%s/%s\",\n",
            utstring_body(bzl_switch_pfx),
            "ocaml");
    fprintf(bootstrap_FILE, "    )\n\n");

    /* we also always have lib/stublibs, which is not an OPAM pkg */
    fprintf(bootstrap_FILE, "    native.local_repository(\n");
    fprintf(bootstrap_FILE, "        name       = \"stublibs\",\n");

    fprintf(bootstrap_FILE, "        path       = ");
    fprintf(bootstrap_FILE, "\"%s/%s\",\n",
            utstring_body(bzl_switch_pfx),
            "lib/stublibs");
    fprintf(bootstrap_FILE, "    )\n\n");
}

/*
  precondition: bzl_switch_pfx set to here or xdg coswitch
  bootstrap_FILE: remains open so opam pkgs can write to it
 */
EXPORT void emit_ocaml_workspace(char *bzl_switch_lib)  // dest
{
#if defined(TRACING)
    if (mibl_trace) log_trace(BLU "emit_ocaml_workspace:" CRESET " %s",
                         bzl_switch_lib);
#endif
    /* printf("emit_ocaml_repo, bzl_switch_pfx: %s\n", */
    /*        utstring_body(bzl_switch_pfx)); */

    /* If we're writing bazel stuff directly into an opam switch, we
       do not need to write a BOOTSTRAP.bzl file here. We can always
       emit one for a list of pkgs since we know where the workspaces
       are. */
    /* emit_ocaml_bootstrap(opam_switch_lib, bootstrap_FILE); */

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_lib); // pfx); */
    utstring_printf(ocaml_file, "%s/ocaml", bzl_switch_lib);
    mkdir_r(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/WORKSPACE.bazel");

    if (verbose && verbosity > 1)
        log_info("Writing ws file: %s", utstring_body(ocaml_file));

    FILE *ostream = fopen(utstring_body(ocaml_file), "w");
    if (ostream == NULL) {
        log_error("fopen: %s: %s", strerror(errno),
                  utstring_body(ocaml_file));
        fprintf(stderr, "fopen: %s: %s", strerror(errno),
                utstring_body(ocaml_file));
        fprintf(stderr, "exiting\n");
        /* perror(utstring_body(ocaml_file)); */
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "workspace( name = \"ocaml\" )"
            "    # generated file - DO NOT EDIT\n");

    fclose(ostream);

    /* now drop WORKSPACE.bazel from path */
    utstring_renew(ocaml_file);
    /* utstring_concat(ocaml_file, bzl_switch_lib); // pfx); */
    utstring_printf(ocaml_file, "%s/ocaml", bzl_switch_lib);

    emit_ocaml_bin_dir(bzl_switch_lib);

    emit_ocaml_platform_buildfiles(bzl_switch_lib);
    emit_ocaml_toolchain_buildfiles(bzl_switch_lib);
    /* /\* X emit_ocaml_toolchain_buildfile(); *\/ */

    /* /\* FIXME: decide on stdlib or runtime *\/ */
    /* /\* X emit_ocaml_stdlib_pkg(opam_switch_lib); *\/ */
    emit_ocaml_runtime_pkg(bzl_switch_lib);

    emit_ocaml_stublibs(bzl_switch_lib);
    emit_lib_stublibs(bzl_switch_lib);

    emit_compiler_libs_pkg(bzl_switch_lib);
    emit_ocaml_compiler_libs_pkg(bzl_switch_lib);

    emit_ocaml_bigarray_pkg(bzl_switch_lib);
    emit_ocaml_dynlink_pkg(bzl_switch_lib);
    emit_ocaml_num_pkg(bzl_switch_lib);
    emit_ocaml_ocamldoc_pkg(bzl_switch_lib);
    emit_ocaml_str_pkg(bzl_switch_lib);
    emit_ocaml_threads_pkg(bzl_switch_lib);
    /* // vmthreads removed in v. 4.08.0? */
    emit_ocaml_unix_pkg(bzl_switch_lib);

    emit_ocaml_c_api_pkg(bzl_switch_lib);

    // stdlib?
    // version-dependent: bigarray, num, raw_spacetime
}
