#include <stdbool.h>

#include "log.h"
#include "utarray.h"
#include "utstring.h"

#include "utils.h"
#include "findlib.h"

#include "emit_jsoo.h"

extern int spfactor;
extern char *sp;

#if defined(DEVBUILD)
extern bool mibl_debug;
#endif

/* emit_bazel_jsoo:
 * jsoo_library(
 *    name = "js",
 *    srcs = ["base.cma"],
 *    deps = [
 *        "@base//lib/base_internalhash_types:js",
 *        "@base//lib/caml:js",
 *        "@base//lib/shadow_stdlib:js",
 *        "@sexplib0//lib/sexplib0:js",
 *    ],
 * )
 * jsoo_import(
 *    name = "runtime",
 *    srcs = ["runtime.js"]
 * )
 */
void emit_bazel_jsoo(FILE* ostream,
                     int level, /* indentation control */
                     /* char *_repo, */
                     /* char *_pkg_path, */
                     char *_pkg_prefix,
                     char *_pkg_name,
                     /* for constructing import label: */
                     char *_filedeps_path, /* _lib */
                     /* char *_subpkg_dir, */
                     obzl_meta_entries *_entries,
                     char *property, /* = 'archive' or 'plugin' */
                     obzl_meta_package *_pkg)
{
    (void)_pkg;
    (void)_filedeps_path;
#if defined(TRACING)
    log_debug("EMIT_BAZEL_JSOO _pkg_name: '%s'; prop: '%s'; filedeps path: '%s'",
              _pkg_name, property, _filedeps_path);
    log_debug("  pkg_prefix: %s", _pkg_prefix);

    /* static UT_string *archive_srcfile; // FIXME: dealloc? */

    if (mibl_debug)
        log_debug("_filedeps_path: '%s'", _filedeps_path);
#endif

    fprintf(ostream, "\njsoo_library(\n");
    fprintf(ostream, "    name = \"js\",\n");

    /* FIXME: this code is copied from emit_bazel_archive_attr;
       it writes the 'srcs = ["foo.cma"]' attribute.
       TODO: consolidate them so we only do this once.
    */

    struct obzl_meta_property *deps_prop = obzl_meta_entries_property(_entries, property);
    if ( deps_prop == NULL ) {
        log_warn("Prop '%s' not found: %s.", property); //, pkg_name);
        return;
    }

    obzl_meta_settings *settings = obzl_meta_property_settings(deps_prop);

    int settings_ct = obzl_meta_settings_count(settings);
#if defined(TRACING)
    log_info("settings count: %d", settings_ct);
#endif
    if (settings_ct == 0) {
#if defined(TRACING)
        log_info("No settings for %s", obzl_meta_property_name(deps_prop));
#endif
        return;
    }
    UT_string *cmtag;  /* cma or cmxa */
    utstring_new(cmtag);

    obzl_meta_setting *setting = NULL;

    obzl_meta_values *vals;
    obzl_meta_value *archive_name = NULL;

    for (int i = 0; i < settings_ct; i++) {
        setting = obzl_meta_settings_nth(settings, i);
#if defined(TRACING)
        log_debug("setting[%d]", i+1);
#endif
        /* dump_setting(0, setting); */

        obzl_meta_flags *flags = obzl_meta_setting_flags(setting);

        /* skip any setting with deprecated flag, e.g. 'vm' */
        if (obzl_meta_flags_deprecated(flags)) {
            /* log_debug("OMIT attribute with DEPRECATED flag"); */
            continue;
        }

        /* if (flags != NULL) register_flags(flags); */

        /* if (g_ppx_pkg) {        /\* set by opam_bootstrap.handle_lib_meta *\/ */
        /* } */

        bool has_conditions = false;
        /* if (flags == NULL) */
        /*     utstring_printf(cmtag, "//conditions:default"); */
        /* else */
        /* updates cmtag */
        has_conditions = obzl_meta_flags_to_cmtag(flags, cmtag);

        if (!has_conditions) {
            /* error? */
            goto _exit;          /* continue does not seem to exit loop */
        }
        if (strncmp(utstring_body(cmtag), "cma", 3) != 0)
            /* we only want cma archive */
            continue;

        if (settings_ct > 1) {
            fprintf(ostream, "%*s%s", level*spfactor, sp, "srcs");
        }

        /* now we handle UPDATE settings */
        vals = resolve_setting_values(setting, flags, settings);
#if defined(TRACING)
        log_debug("setting values:");
#endif
        /* dump_values(0, vals); */

        /* now we construct a bazel label for each value */
        UT_string *label;
        utstring_new(label);

        for (int j = 0; j < obzl_meta_values_count(vals); j++) {
            archive_name = obzl_meta_values_nth(vals, j);
#if defined(TRACING)
            log_info("prop[%d] '%s' == '%s'",
                     j, property, (char*)*archive_name);
#endif
            utstring_clear(label);
            if (_pkg_prefix == NULL) {
                utstring_printf(label,
                                "%s",
                                /* "@%s//%s", // filedeps path: %s", */
                                /* /\* _pkg_name, *\/ */
                                /* _filedeps_path, */
                                *archive_name);
            } else {
                char *start = strchr(_pkg_prefix, '/');
                /* int repo_len = start - (char*)_pkg_prefix; */
                if (start == NULL) {
                    utstring_printf(label,
                                    "%s",
                                    /* "@%.*s//%s:%s", // || PKG_pfx: %s", */
                                    /* repo_len, */
                                    /* _pkg_prefix, */
                                    /* _pkg_name, */
                                    *archive_name);
                    /* _pkg_prefix); */
                } else {
                    start++;
                    utstring_printf(label,
                                    "%s",
                                    /* "@%.*s//%s/%s:%s", */
                                    /* repo_len, */
                                    /* _pkg_prefix, */
                                    /* (char*)start, */
                                    /* _pkg_name, */
                                    *archive_name);
                    /* _pkg_prefix); */
                }

                /* fprintf(ostream, "%*s\"@%.*s//%s\",\n", */
                /*         (1+level)*spfactor, sp, */
                /*         repo_len, */
                /*         *v, */
                /*         start+1); */

            }
            // FIXME: verify existence using access()?
            int indent = 3;
            if (strncmp(utstring_body(cmtag), "cma", 3) == 0)
                indent--;
            if (settings_ct > 1) {
                    fprintf(ostream, "%*s = [\"%s\"],\n",
                                        /* "@%.*s//%s/%s:%s", */
                            indent, sp, utstring_body(label));
            } else {
                    fprintf(ostream, "%*s = [\"%s\"],\n",
                            indent, sp, utstring_body(label));
            }
        }
        utstring_free(label);
    /* _next: ; */
        ;
    }
    /* emit 'deps = [...]' attribute */
    emit_bazel_deps_attribute(ostream, 1,
                              true, /* jsoo */
                              ocaml_ws, /* repo */
                              "lib",    /* pkg */
                              _pkg_name, /* pkg_name */
                              _entries);

    /* no need for ppx_codeps, they're only needed for ocaml compilation */

 _exit: ;
    /* fprintf(ostream, "    visibility = [\"//visibility:public\"]\n"); */
    fprintf(ostream, ")\n");

    return;
}

