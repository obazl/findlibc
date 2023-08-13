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
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <unistd.h>

#include "log.h"
#include "utarray.h"
#include "utstring.h"

#include "findlib.h"
#include "parse_test.h"

UT_string *build_file;

UT_string *buffer;

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

    log_info("not skipping");
    return false;
}

int compareFiles(FILE *file1, FILE *file2)
{
    /* printf("comparing files\n"); */
    char ch1 = getc(file1);
    char ch2 = getc(file2);
    int error = 0;
    (void)error;
    int pos = 0, line = 1;
    while (ch1 != EOF && ch2 != EOF){
        pos++;
        if (ch1 == '\n' && ch2 == '\n'){
            line++;
            pos = 0;
        }
        if (ch1 != ch2){
            error++;
            printf("File mismatch at (%d:%d)\n", line, pos);
            return -1;
        }
        ch1 = getc(file1);
        ch2 = getc(file2);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int opt;
    UT_string *meta_file;
    utstring_new(meta_file);

    while ((opt = getopt(argc, argv, "f:hv")) != -1) {
        switch (opt) {
        case 'f':
            /* BUILD.bazel or BUILD file */
            utstring_printf(meta_file, "%s", optarg);
            break;
        case 'h':
            log_info("Help: ");
            exit(EXIT_SUCCESS);
        default:
            log_error("Usage: %s -f /path/to/META", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (utstring_len(meta_file) == 0) {
        log_error("-- -f /path/to/META.file must be provided.");
        exit(EXIT_FAILURE);
    }

    char *wd = getenv("BUILD_WORKING_DIRECTORY");
    if (wd) {
        /* we launched from bazel workspace, cd to launch dir */
        chdir(wd);
    }

    struct obzl_meta_package *pkg = obzl_meta_parse_file(utstring_body(meta_file));
    if (pkg == NULL) {
        if (errno == -1)
            log_info("Empty META file: %s", utstring_body(meta_file));
        else
            if (errno == -2)
                log_info("META file contains only whitespace: %s", utstring_body(meta_file));
            else
                log_error("Error parsing %s", utstring_body(meta_file));
        /* emitted_bootstrapper = false; */
    } else {
#if defined(TRACING)
        log_info("PARSED %s", utstring_body(meta_file));
        DUMP_PKG(0, pkg);

        /* if (mibl_debug_findlib) */
        /*     dump_package(0, pkg); */
#endif
        /* return 0; */

        /* stdlib_root = false; */

        if (obzl_meta_entries_property(pkg->entries, "library_kind")) {
            /* special handling for ppx packages */
            log_debug("handling ppx package: %s", obzl_meta_package_name(pkg));
            /* g_ppx_pkg = true; */
            /* emit_build_bazel_ppx(obazl_opam_root, host_repo, "lib", "", pkg); */
        } else {
            log_debug("handling normal package: %s", obzl_meta_package_name(pkg));
            /* g_ppx_pkg = true; */
        }

        /* skip ocaml core pkgs, we do them by hand */
        if (_skip_pkg(utstring_body(meta_file))) {
            log_debug("SKIPPING pkg %s", utstring_body(meta_file));
            return 0;
        }

        /* emit_new_local_pkg_repo(bootstrap_FILE, /\* _pkg_prefix, *\/ */
        /*                         pkg); */
        /* fflush(bootstrap_FILE); */
        /* emit_local_repo_decl(bootstrap_FILE, pkg->name); */
        /* emitted_bootstrapper = true; */

        /* fflush(bootstrap_FILE); */

        UT_string *imports_path;
        utstring_new(imports_path);
        /* utstring_printf(imports_path, "_lib/%s", */
        utstring_printf(imports_path, "%s",
                        obzl_meta_package_name(pkg));
        log_debug("imports_path: %s", utstring_body(imports_path));
        /* log_debug("emitting for pkg: %s", pkg->name); */
        /* dump_package(0, pkg); */
        /* fflush(bootstrap_FILE); */

        /* utstring_renew(workspace_file); */
        /* /\* utstring_printf(workspace_file, "%s/%s", obazl_opam_root, pkg->name); *\/ */
        /* mkdir_r(utstring_body(workspace_file)); */
        /* utstring_printf(workspace_file, "%s", "/WORKSPACE.bazel"); */
        /* printf("emitting ws file: %s\n", utstring_body(workspace_file)); */
        /* emit_workspace_file(workspace_file, pkg->name); */

        /* utstring_renew(pkg_parent); */
    }
}
