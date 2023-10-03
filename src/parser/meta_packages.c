#include <stdbool.h>

/* #include "log.h" */
#if EXPORT_INTERFACE
#include "semver.h"
#endif
#include "utarray.h"
#include "utstring.h"
#include "utstrsort.h"

#include "meta_packages.h"

#if EXPORT_INTERFACE
#include "uthash.h"

#if defined(DEBUG_fastbuild)
extern int  findlib_debug;
extern bool findlib_trace;
#endif

struct obzl_meta_package {
    char *name;                 /* uthash key */
    char *module_name;          /* Bazel: lowercase */
    char *path;
    char *directory;            /* subdir */
    char *metafile;
    obzl_meta_entries *entries;          /* list of struct obzl_meta_entry */
    // we CAN use this as a uthash, but we don't have to
    UT_hash_handle hh;
};
#endif

EXPORT char *obzl_meta_package_name(obzl_meta_package *_pkg)
{
    return _pkg->name;
}

EXPORT char *obzl_meta_package_dir(obzl_meta_package *_pkg)
{
    return _pkg->directory;
}

EXPORT char *obzl_meta_package_directory_prop(obzl_meta_package *_pkg)
{
    /* char *d = obzl_meta_directory_property(_pkg->entries); */

    return _pkg->directory;
}

EXPORT char *obzl_meta_package_src(obzl_meta_package *_pkg)
{
    return _pkg->metafile;
}

EXPORT obzl_meta_entries *obzl_meta_package_entries(obzl_meta_package *_pkg)
{
    return _pkg->entries;
}

/* **************************************************************** */
EXPORT int obzl_meta_package_subpkg_count(obzl_meta_package *_pkg)
{
    /* obzl_meta_entries *entries = _pkg->entries; */
    obzl_meta_entry *e = NULL;
    /* obzl_meta_package *subpkg = NULL; */

    int pkg_ct = 0;

    for (int i = 0; i < obzl_meta_entries_count(_pkg->entries); i++) {
        e = obzl_meta_entries_nth(_pkg->entries, i);
        if (e->type == OMP_PACKAGE) {
            pkg_ct++;
            /* subpkg = e->package; */
            pkg_ct += obzl_meta_package_subpkg_count(e->package);
        }
    }
    return pkg_ct;
}

/* **************************************************************** */
EXPORT bool obzl_meta_package_has_archives(obzl_meta_package *_pkg)
{
    TRACE_ENTRY
    //FIXME: use a has_archives flag

    /* obzl_meta_entries *entries = _pkg->entries; */
    obzl_meta_entry *e = NULL;
    for (int i = 0; i < obzl_meta_entries_count(_pkg->entries); i++) {
        e = obzl_meta_entries_nth(_pkg->entries, i);
        if (e->type == OMP_PROPERTY) {
            if (strncmp(e->property->name, "archive", 7) == 0) {
                return true;
            }
        }
    }
    return false;
}

EXPORT bool obzl_meta_package_has_plugins(obzl_meta_package *_pkg)
{
    //FIXME: use a has_plugins flag

    /* obzl_meta_entries *entries = _pkg->entries; */
    obzl_meta_entry *e = NULL;
    for (int i = 0; i < obzl_meta_entries_count(_pkg->entries); i++) {
        e = obzl_meta_entries_nth(_pkg->entries, i);
        if (e->type == OMP_PROPERTY) {
            if (strncmp(e->property->name, "plugin", 7) == 0) {
                return true;
            }
        }
    }
    return false;
}

EXPORT bool obzl_meta_package_has_subpackages(obzl_meta_package *_pkg)
{
    //FIXME: use a has_subpackages flag
    LOG_DEBUG(0, "%s", "obzl_meta_package_has_subpackages");
    obzl_meta_entries *entries = _pkg->entries;
    obzl_meta_entry *e = NULL;

    for (int i = 0; i < obzl_meta_entries_count(entries); i++) {
        e = obzl_meta_entries_nth(_pkg->entries, i);
        LOG_DEBUG(0, "entry type: %d", e->type);
        if (e->type == OMP_PROPERTY) {
            LOG_DEBUG(0, "Property entry: %s", e->property->name);
        } else {
            if (e->type == OMP_PACKAGE) {
                LOG_DEBUG(0, "Package entry: %s", e->package->name);
                return true;
            }
        }
    }
    return false;
}

EXPORT obzl_meta_property *obzl_meta_package_property(obzl_meta_package *_pkg, char *_name)
{
    TRACE_ENTRY;
    TRACE_LOG("  prop name: %s", _name);

    /* utarray_find requires a sort; not worth the cost */
    obzl_meta_entry *e = NULL;
    for (int i = 0; i < obzl_meta_entries_count(_pkg->entries); i++) {
        e = obzl_meta_entries_nth(_pkg->entries, i);
        if (e->type == OMP_PROPERTY) {
            if (strncmp(e->property->name, _name, 256) == 0) {
                TRACE_EXIT;
                return e->property;
            }
        }
        /* LOG_DEBUG(0, "iteration %d", i); */
    }
    TRACE_EXIT;
    return NULL;
}

EXPORT obzl_meta_values *resolve_setting_values(obzl_meta_setting *_setting,
                                         obzl_meta_flags *_flags,
                                         obzl_meta_settings *_settings)
{
    TRACE_ENTRY
/*         LOG_DEBUG(0, "resolve_setting_values, opcode: %d", _setting->opcode); */
    obzl_meta_values * vals = obzl_meta_setting_values(_setting);
    /* LOG_DEBUG(0, "vals ct: %d", obzl_meta_values_count(vals)); */
    if (_setting->opcode == OP_SET)
        return vals;

    /* else OP_UPDATE */

    UT_array *resolved_values;
    utarray_new(resolved_values, &ut_str_icd);
    utarray_concat(resolved_values, vals->list);

    /* for each flag, search settings for matching flag */
    int settings_ct = obzl_meta_settings_count(_settings);
    struct obzl_meta_setting *a_setting;

    int flags_ct    = obzl_meta_flags_count(_flags);
    /* printf("\tflags_ct: %d\n", flags_ct); */
    struct obzl_meta_flag *a_flag = NULL;

    for (int i=0; i < flags_ct; i++) {
        a_flag = obzl_meta_flags_nth(_flags, i);
        for (int j=0; j < settings_ct; j++) {
            a_setting = obzl_meta_settings_nth(_settings, j);
            if (a_setting == _setting) continue; /* don't match self */

            obzl_meta_flags *setting_flags = obzl_meta_setting_flags(a_setting);

            if (setting_flags == NULL) {
                /* always match no flags, e.g. 'requires = "findlib.internal"' */
                obzl_meta_values *vs = obzl_meta_setting_values(a_setting);
                /* printf("xconcatenating\n"); fflush(stdout); fflush(stderr); */
                utarray_concat(resolved_values, vs->list);
                continue;
            }

            int ct = obzl_meta_flags_count(setting_flags);
            if (ct > 1) {
                /* only try to match singletons? */
                continue;
            }

            obzl_meta_flag *setting_flag = obzl_meta_flags_nth(setting_flags, 0);
            if (setting_flag->polarity == a_flag->polarity) {
                if (strncmp(setting_flag->s, a_flag->s, 32) == 0) {
                    LOG_DEBUG(0, "%s", "matched flag");
                    /* we have found a setting with exactly one flag, that matches the search flag */
                    /* now we check the setting's opcode - it should always be SET? */
                    /* then add its values list */
                    obzl_meta_values *vs = obzl_meta_setting_values(a_setting);
                    utarray_concat(resolved_values, vs->list);
                    /* dump_setting(4, a_setting); */
                }
            }
        }
    }
    obzl_meta_values *new_values = (obzl_meta_values*)calloc(sizeof(obzl_meta_values),1);
    new_values->list = resolved_values;
    return new_values;
}

/* **************** */
EXPORT int pkg_deps(struct obzl_meta_package *_pkg,
                    UT_array *pending_deps,
                    UT_array *completed_deps)
{
    TRACE_ENTRY
    utarray_sort(completed_deps,strsort);

    obzl_meta_entries *entries = obzl_meta_package_entries(_pkg);

    char *property = "requires";
    struct obzl_meta_property *deps_prop = NULL;
    deps_prop = obzl_meta_entries_property(entries, property);
    if ( deps_prop == NULL ) {
        LOG_WARN(0, "Prop '%s' not found for pkg: %s.", property, _pkg->name);
        return 0;
    }

    obzl_meta_settings *settings = obzl_meta_property_settings(deps_prop);
    obzl_meta_setting *setting = NULL;

    int settings_ct = obzl_meta_settings_count(settings);
    if (settings_ct == 0) {
        LOG_INFO(0, "No settings for %s", obzl_meta_property_name(deps_prop));
        return 0;
    } else {
        /* LOG_INFO(0, "settings count: %d", settings_ct); */
    }

    int settings_no_ppx_driver_ct = obzl_meta_settings_flag_count(settings, "ppx_driver", false);

    settings_ct -= settings_no_ppx_driver_ct;

    /* LOG_INFO(0, "settings count w/o ppx_driver: %d", settings_ct); */

    if (settings_ct == 0) {
        LOG_INFO(0, "No deps for %s", obzl_meta_property_name(deps_prop));
        return 0;
    }

    obzl_meta_values *vals;
    obzl_meta_value *dep_name = NULL;

    UT_string *condition_name;
    utstring_new(condition_name);

    LOG_DEBUG(0, "iterating settings", "");
    for (int i = 0; i < settings_ct; i++) {
        setting = obzl_meta_settings_nth(settings, i);
        /* LOG_DEBUG(0, "setting %d", i+1); */

        obzl_meta_flags *flags = obzl_meta_setting_flags(setting);
        /* int flags_ct; // = 0; */
        if (flags != NULL) {
            /* register_flags(flags); // why? */
            //int flags_ct =   /* -Wunused-variable */
            obzl_meta_flags_count(flags);
            /* LOG_DEBUG(0, "flags_ct: %d", flags_ct); */
        }

        if (obzl_meta_flags_has_flag(flags, "ppx_driver", false)) {
            continue;
        }

        bool has_conditions;
        if (flags == NULL)
            utstring_printf(condition_name, "//conditions:default");
        else {
            has_conditions = obzl_meta_flags_to_selection_label(flags, condition_name);
            (void)has_conditions; /* prevent [-Wunused-but-set-variable] */
        }

        char *condition_comment = obzl_meta_flags_to_comment(flags);
        /* LOG_DEBUG(0, "condition_comment: %s", condition_comment); */

        /* 'requires' usually has no flags; when it does, empirically we find only */
        /*   ppx pkgs: ppx_driver, -ppx_driver */
        /*   pkg 'batteries': requires(mt) */
        /*   pkg 'num': requires(toploop) */
        /*   pkg 'findlib': requires(toploop), requires(create_toploop) */

        /* Multiple settings on 'requires' means multiple flags; */
        /* empirically, this only happens for ppx packages, typically as */
        /* requires(-ppx_driver,-custom_ppx) */
        /* the (sole?) exception is */
        /*   pkg 'threads': requires(mt,mt_vm), requires(mt,mt_posix) */

        /* if (settings_ct > 1) { */
        /*     fprintf(ostream, "%*s\"X%s%s\": [ ## predicates: %s\n", */
        /*             (1+level)*spfactor, sp, */
        /*             utstring_body(condition_name), */
        /*             (has_conditions)? "" : "", */
        /*             condition_comment); */
        /* } */

        vals = obzl_meta_setting_values(setting);
        /* vals = resolve_setting_values(setting, flags, settings); */
        /* vals = obzl_meta_setting_values(setting); */
        /* LOG_DEBUG(0, "vals ct: %d", obzl_meta_values_count(vals)); */
        /* dump_values(0, vals); */
        /* now we handle UPDATE settings */

/*         LOG_DEBUG(0, "iterating values"); */
        char **p = NULL;        /* for searching completed_deps */
        for (int j = 0; j < obzl_meta_values_count(vals); j++) {
            dep_name = obzl_meta_values_nth(vals, j);
            /* LOG_DEBUG(0, "property val[%d]: '%s'", j, *dep_name); */

            char *s = strdup((char*)*dep_name);
            LOG_DEBUG(2, "DEP: '%s'", s);

            /* FIXME: drop trailing dot segs - only record topelevels */
            char *dot = strchr(s, '.');
            LOG_DEBUG(2, "DOT: %s", dot);

            if (dot) {
                *dot = '\0';
            }
            LOG_DEBUG(2, "DEP DESEGMENTED: '%s'", s);

            p = NULL;
            p = (char**)utarray_find(completed_deps, &s, strsort);
            if (p != NULL) {
                LOG_DEBUG(2, " found completed dep: %s", *p);
            } else {
                utarray_sort(pending_deps,strsort); /* FIXME: can we avoid this? */
                p = (char**)utarray_find(pending_deps, &s, strsort);
                if (p != NULL) {
                    LOG_DEBUG(2, " pending dep already added: %s", *p);
                } else {
                    LOG_DEBUG(2, " adding new pending dep: %s", s);
                    utarray_push_back(pending_deps, &s);
                }
            }
        }
        free(condition_comment);
    }
    return 0;
}

EXPORT semver_t *findlib_pkg_version(struct obzl_meta_package *_pkg)
{
    TRACE_ENTRY;
    //FIXME: ocaml-compiler-libs META shows no version for
    //topleve, but each subpkg has a version string.
    semver_t *semversion
        = (semver_t*)malloc(sizeof(semver_t));
    obzl_meta_value version;
    char *pname = "version";
    struct obzl_meta_property *version_prop
        = obzl_meta_package_property(_pkg, pname);
    if (version_prop) {
        version = obzl_meta_property_value(version_prop);
        /* LOG_DEBUG(0, "    version: %s", (char*)version); */
        if (!semver_is_valid(version)) {
            /* LOG_WARN(0, "BAD VERSION STRING: %s", (char*)version); */
            /* return; */
        } else {
            if (version[0] == 'v') {
                semver_parse(version+1, semversion);
            } else {
                semver_parse(version, semversion);
            }
            /* LOG_DEBUG(0, "    compat: %d", semversion->major); */
            /* semversion.major); */
        }
    } else {
        /* LOG_WARN(0, "    version: 0.0.0 (default)"); */
        /* version = "0.0.0"; */
        /* return; //FIXME ??? e.g. dune META has no version */
    }
    /* return (char*)version; */
    TRACE_EXIT;
    return semversion;
}

/** @brief Returns all predicate-free "required" pkgs.

    @param subpkgs true means also include direct deps
    of all subpkgs as well

 */
EXPORT UT_array *findlib_pkg_deps(struct obzl_meta_package *_pkg,
                                  bool subpkgs)
{
    TRACE_ENTRY;
    /* DUMP_PKG(0, _pkg); */
    TRACE_LOG("  pkg name: %s", _pkg->name);

    UT_array *pkg_deps;
    utarray_new(pkg_deps, &ut_str_icd);

    /* char *s = "testdep"; */
    /* utarray_push_back(pkg_deps, &s); */

    obzl_meta_entries *entries = obzl_meta_package_entries(_pkg);

    // obzl_meta_entries->list is UT_array of obzl_meta_entry
    obzl_meta_entry *p;
    for(p=(obzl_meta_entry*)utarray_front(entries->list);
        p!=NULL;
        p=(obzl_meta_entry*)utarray_next(entries->list,p)) {
        if (p->type == OMP_PACKAGE) {
            /* LOG_DEBUG(0, "  pkg entry: %s", p->package->name); */
            findlib_subpkg_deps(p->package, subpkgs, pkg_deps);
        }
  }

    char *property = "requires";
    struct obzl_meta_property *deps_prop = NULL;
    deps_prop = obzl_meta_entries_property(entries, property);
    if ( deps_prop == NULL ) {
        LOG_WARN(0, "Prop '%s' not found for pkg: %s.", property, _pkg->name);
        return pkg_deps;
    }

    obzl_meta_settings *settings = obzl_meta_property_settings(deps_prop);
    obzl_meta_setting *setting = NULL;

    int settings_ct = obzl_meta_settings_count(settings);
    if (settings_ct == 0) {
        /* LOG_INFO(0, "No settings for %s", obzl_meta_property_name(deps_prop)); */
        return pkg_deps;
    } else {
        /* LOG_INFO(0, "settings count: %d", settings_ct); */
    }

    int settings_no_ppx_driver_ct = obzl_meta_settings_flag_count(settings, "ppx_driver", false);

    settings_ct -= settings_no_ppx_driver_ct;

    /* LOG_INFO(0, "settings count w/o ppx_driver: %d", settings_ct); */

    if (settings_ct == 0) {
        /* LOG_INFO(0, "No deps for %s", obzl_meta_property_name(deps_prop)); */
        return pkg_deps;
    }

    obzl_meta_values *vals;
    obzl_meta_value *dep_name = NULL;

    UT_string *condition_name;
    utstring_new(condition_name);

    LOG_DEBUG(3, "iterating settings", "");
    for (int i = 0; i < settings_ct; i++) {
        setting = obzl_meta_settings_nth(settings, i);
        LOG_DEBUG(3, "setting %d", i+1);
#if defined(DEBUG_fastbuild)
        if (findlib_debug > 3) dump_setting(0, setting);
#endif
        obzl_meta_flags *flags = obzl_meta_setting_flags(setting);
        /* int flags_ct = 0; */
        if (flags != NULL) {
            /* register_flags(flags); // why? */
            //int flags_ct =   /* -Wunused-variable */
            obzl_meta_flags_count(flags);
            /* LOG_DEBUG(0, "flags_ct: %d", flags_ct); */
        }

        if (obzl_meta_flags_has_flag(flags, "ppx_driver", false)) {
            continue;
        }

        /* bool has_conditions; */
        /* if (flags == NULL) */
        /*     utstring_printf(condition_name, "//conditions:default"); */
        /* else { */
        /*     has_conditions = obzl_meta_flags_to_selection_label(flags, condition_name); */
        /*     (void)has_conditions; /\* prevent [-Wunused-but-set-variable] *\/ */
        /* } */

        /* char *condition_comment = obzl_meta_flags_to_comment(flags); */
        /* LOG_DEBUG(0, "condition_comment: %s", condition_comment); */

        /* 'requires' usually has no flags; when it does, empirically we find only */
        /*   ppx pkgs: ppx_driver, -ppx_driver */
        /*   pkg 'batteries': requires(mt) */
        /*   pkg 'num': requires(toploop) */
        /*   pkg 'findlib': requires(toploop), requires(create_toploop) */

        /* Multiple settings on 'requires' means multiple flags; */
        /* empirically, this only happens for ppx packages, typically as */
        /* requires(-ppx_driver,-custom_ppx) */
        /* the (sole?) exception is */
        /*   pkg 'threads': requires(mt,mt_vm), requires(mt,mt_posix) */

        /* if (settings_ct > 1) { */
        /*     fprintf(ostream, "%*s\"X%s%s\": [ ## predicates: %s\n", */
        /*             (1+level)*spfactor, sp, */
        /*             utstring_body(condition_name), */
        /*             (has_conditions)? "" : "", */
        /*             condition_comment); */
        /* } */

        vals = obzl_meta_setting_values(setting);
        /* vals = resolve_setting_values(setting, flags, settings); */
        /* vals = obzl_meta_setting_values(setting); */
        LOG_DEBUG(3, "vals ct: %d", obzl_meta_values_count(vals));
#if defined(DEBUG_fastbuild)
        if (findlib_debug > 3) dump_values(0, vals);
#endif
        /* now we handle UPDATE settings */

        LOG_DEBUG(3, "iterating values", "");
        char **p = NULL;        /* for searching */
        for (int j = 0; j < obzl_meta_values_count(vals); j++) {
            dep_name = obzl_meta_values_nth(vals, j);
            LOG_DEBUG(3, "property val[%d]: '%s'", j, *dep_name);

            char *s = strdup((char*)*dep_name);
            LOG_DEBUG(3, "DEP: '%s'", s);
            /* utarray_push_back(pkg_deps, &s); */

            /* utarray_push_back(pkg_deps, &s); */

            /* FIXME: drop trailing dot segs - only record toplevels */
            char *dot = strchr(s, '.');
            LOG_DEBUG(3, "DOT: %s", dot);

            if (dot) {
                *dot = '\0';
            }

            /* LOG_DEBUG(0, "DEP DESEGMENTED: '%s'", s); */

            /* if (subpkgs) { */
            /*     utarray_push_back(pkg_deps, (char**)dep_name); */
            /* } else { */
            if (strcmp(s, _pkg->name) != 0) {
                p = NULL;
                /* FIXME: can we avoid this sort? */
                utarray_sort(pkg_deps,strsort);
                p = (char**)utarray_find(pkg_deps, &s, strsort);
                if (p == NULL) {
                   utarray_push_back(pkg_deps, &s);
                }
            }
            /* } */
/*                 utarray_sort(pending_deps,strsort); /\* FIXME: can we avoid this? *\/ */
/*                 p = (char**)utarray_find(pending_deps, &s, strsort); */
/*                 if (p != NULL) { */
/*                         LOG_DEBUG(0, " pending dep already added: %s", *p); */
/*                 } else { */
/*                     LOG_DEBUG(0, " adding new pending dep: %s", s); */
/*                     utarray_push_back(pending_deps, &s); */
/*                 } */
/*             } */
        }
        /* free(condition_comment); */
    }
    TRACE_EXIT;
    return pkg_deps;
}

EXPORT UT_array *findlib_subpkg_deps(struct obzl_meta_package *_pkg,
                                     bool subpkgs,
                                     UT_array *pkg_deps)
{
    TRACE_ENTRY;
    /* DUMP_PKG(0, _pkg); */

    /* UT_array *pkg_deps; */
    /* utarray_new(pkg_deps, &ut_str_icd); */

    /* char *s = "testdep"; */
    /* utarray_push_back(pkg_deps, &s); */

    obzl_meta_entries *entries = obzl_meta_package_entries(_pkg);

    // obzl_meta_entries->list is UT_array of obzl_meta_entry
    /* LOG_DEBUG(0, "findlib_subpkg_deps: %s", _pkg->name); */

    obzl_meta_entry *p;
    for(p=(obzl_meta_entry*)utarray_front(entries->list);
        p!=NULL;
        p=(obzl_meta_entry*)utarray_next(entries->list,p)) {
        if (p->type == OMP_PACKAGE) {
            /* LOG_DEBUG(0, "  pkg entry: %s", p->package->name); */
            findlib_subpkg_deps(p->package, subpkgs, pkg_deps);
        }
  }

    char *property = "requires";
    struct obzl_meta_property *deps_prop = NULL;
    deps_prop = obzl_meta_entries_property(entries, property);
    if ( deps_prop == NULL ) {
        LOG_WARN(0, "Prop '%s' not found for pkg: %s.", property, _pkg->name);
        return pkg_deps;
    }

    obzl_meta_settings *settings = obzl_meta_property_settings(deps_prop);
    obzl_meta_setting *setting = NULL;

    int settings_ct = obzl_meta_settings_count(settings);
    if (settings_ct == 0) {
        LOG_INFO(3, "No settings for %s", obzl_meta_property_name(deps_prop));
        return pkg_deps;
    } else {
        LOG_INFO(3, "settings count: %d", settings_ct);
    }

    int settings_no_ppx_driver_ct = obzl_meta_settings_flag_count(settings, "ppx_driver", false);

    settings_ct -= settings_no_ppx_driver_ct;

    LOG_INFO(3, "settings count w/o ppx_driver: %d", settings_ct);

    if (settings_ct == 0) {
        LOG_INFO(3, "No deps for %s", obzl_meta_property_name(deps_prop));
        return pkg_deps;
    }

    obzl_meta_values *vals;
    obzl_meta_value *dep_name = NULL;

    UT_string *condition_name;
    utstring_new(condition_name);

    LOG_DEBUG(3, "iterating settings", "");
    for (int i = 0; i < settings_ct; i++) {
        setting = obzl_meta_settings_nth(settings, i);
#if defined(DEBUG_fastbuild)
        LOG_DEBUG(3, "setting %d", i+1);
        if (findlib_debug > 3) dump_setting(0, setting);
#endif
        obzl_meta_flags *flags = obzl_meta_setting_flags(setting);
        /* int flags_ct = 0; */
        if (flags != NULL) {
            /* register_flags(flags); // why? */
            //int flags_ct =   /* -Wunused-variable */
            obzl_meta_flags_count(flags);
            /* LOG_DEBUG(0, "flags_ct: %d", flags_ct); */
        }

        if (obzl_meta_flags_has_flag(flags, "ppx_driver", false)) {
            continue;
        }

        /* bool has_conditions; */
        /* if (flags == NULL) */
        /*     utstring_printf(condition_name, "//conditions:default"); */
        /* else { */
        /*     has_conditions = obzl_meta_flags_to_selection_label(flags, condition_name); */
        /*     (void)has_conditions; /\* prevent [-Wunused-but-set-variable] *\/ */
        /* } */

        /* char *condition_comment = obzl_meta_flags_to_comment(flags); */
        /* LOG_DEBUG(0, "condition_comment: %s", condition_comment); */

        /* 'requires' usually has no flags; when it does, empirically we find only */
        /*   ppx pkgs: ppx_driver, -ppx_driver */
        /*   pkg 'batteries': requires(mt) */
        /*   pkg 'num': requires(toploop) */
        /*   pkg 'findlib': requires(toploop), requires(create_toploop) */

        /* Multiple settings on 'requires' means multiple flags; */
        /* empirically, this only happens for ppx packages, typically as */
        /* requires(-ppx_driver,-custom_ppx) */
        /* the (sole?) exception is */
        /*   pkg 'threads': requires(mt,mt_vm), requires(mt,mt_posix) */

        /* if (settings_ct > 1) { */
        /*     fprintf(ostream, "%*s\"X%s%s\": [ ## predicates: %s\n", */
        /*             (1+level)*spfactor, sp, */
        /*             utstring_body(condition_name), */
        /*             (has_conditions)? "" : "", */
        /*             condition_comment); */
        /* } */

        vals = obzl_meta_setting_values(setting);
        /* vals = resolve_setting_values(setting, flags, settings); */
        /* vals = obzl_meta_setting_values(setting); */
        LOG_DEBUG(3, "vals ct: %d", obzl_meta_values_count(vals));
#if defined(DEBUG_fastbuild)
        if (findlib_debug > 2) dump_values(0, vals);
#endif
        /* now we handle UPDATE settings */

        LOG_DEBUG(2, "iterating values", "");
        char **p = NULL;        /* for searching */
        for (int j = 0; j < obzl_meta_values_count(vals); j++) {
            dep_name = obzl_meta_values_nth(vals, j);
            LOG_DEBUG(2, "property val[%d]: '%s'", j, *dep_name);

            char *s = strdup((char*)*dep_name);
            LOG_DEBUG(2, "DEP: '%s'", s);
            /* utarray_push_back(pkg_deps, &s); */

            /* utarray_push_back(pkg_deps, &s); */

            /* FIXME: drop trailing dot segs - only record toplevels */
            char *dot = strchr(s, '.');
            LOG_DEBUG(2, "DOT: %s", dot);

            if (dot) {
                *dot = '\0';
            }

            LOG_DEBUG(2, "DEP DESEGMENTED: '%s'", s);

            /* if (subpkgs) { */
            /*     utarray_push_back(pkg_deps, (char**)dep_name); */
            /* } else { */
                p = NULL;
                /* FIXME: can we avoid this sort? */
                utarray_sort(pkg_deps,strsort);
                p = (char**)utarray_find(pkg_deps, &s, strsort);
                if (p == NULL) {
                    utarray_push_back(pkg_deps, &s);
                }
            /* } */
/*                 utarray_sort(pending_deps,strsort); /\* FIXME: can we avoid this? *\/ */
/*                 p = (char**)utarray_find(pending_deps, &s, strsort); */
/*                 if (p != NULL) { */
                        /* LOG_DEBUG(0, " pending dep already added: %s", *p); */
/*                 } else { */
/*                     LOG_DEBUG(0, " adding new pending dep: %s", s); */
/*                     utarray_push_back(pending_deps, &s); */
/*                 } */
/*             } */
        }
        /* free(condition_comment); */
    }
    return pkg_deps;
}
