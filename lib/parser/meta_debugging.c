#include "liblogc.h"
#include "utarray.h"

#include "meta_debugging.h"

int fl_indent = 2;
int fl_delta = 2;
char *fl_sp = " ";

bool debug_meta;

#if EXPORT_INTERFACE
#if defined(PROFILE_fastbuild)
#define DUMP_PKG(fl_indent, pkg) \
    log_debug("FINDLIB PKG: %s", pkg->name); \
    dump_package(fl_indent, pkg)
#else
#define DUMP_PKG(fl_indent, pkg)
#endif
#endif

EXPORT void dump_package(int fl_indent, struct obzl_meta_package *pkg)
{
    log_debug("%*sdump_package:", fl_indent, fl_sp);
    log_debug("%*sname:      %s", fl_delta+fl_indent, fl_sp, pkg->name);
    log_debug("%*sdirectory: %s", fl_delta+fl_indent, fl_sp, pkg->directory);
    log_debug("%*fl_spath: %s", fl_delta+fl_indent, fl_sp, pkg->path);
    log_debug("%*smetafile:  %s", fl_delta+fl_indent, fl_sp, pkg->metafile);
    dump_entries(fl_delta+fl_indent, pkg->entries);
}

EXPORT void dump_entry(int fl_indent, struct obzl_meta_entry *entry)
{
    log_trace("%*sdump_entry:", fl_indent, fl_sp);
    log_debug("%*sentry type: %s", fl_delta+fl_indent, fl_sp,
              entry->type == OMP_PROPERTY
              ? "property"
              : "package");
    if (entry->type == OMP_PROPERTY) {
        dump_property(fl_delta+fl_indent, entry->property);
    } else {
        dump_package(fl_delta+fl_indent, entry->package);
    }
}

EXPORT void dump_entries(int fl_indent, struct obzl_meta_entries *entries)
{
    log_trace("%*sdump_entries()", fl_indent, fl_sp); //, entries);
    if (entries == NULL) {
        log_trace("%*sentries: none", fl_indent, fl_sp);
    } else {
        obzl_meta_entry *e = NULL;
        for (int i = 0; i < obzl_meta_entries_count(entries); i++) {
            e = obzl_meta_entries_nth(entries, i);
            /* log_trace("e: %p", e); */
            /* log_trace("e type: %d", e->type); */
            dump_entry(fl_delta+fl_indent, e);
        }
        /* log_trace("%*sdump_entries() DONE", fl_indent, fl_sp); */
    }
}

EXPORT void dump_flags(int fl_indent, obzl_meta_flags *flags)
{
    fl_indent++;            /* account for width of log label */
    /* log_trace("%*sdump_flags", fl_indent, fl_sp); */
    if (flags == NULL) {
        log_trace("%*sflags: none", fl_indent, fl_sp);
        return;
    } else {
        if ( flags->list ) {
            if (utarray_len(flags->list) == 0) {
                log_trace("%*sflags: none.", fl_indent, fl_sp);
                return;
            } else {
                log_trace("%*sflags ct: %d", fl_indent, fl_sp, utarray_len(flags->list));
                struct obzl_meta_flag *a_flag = NULL;
                while ( (a_flag=(struct obzl_meta_flag*)utarray_next(flags->list, a_flag))) {
                    log_debug("%*s%s (polarity: %d)", fl_delta+fl_indent, fl_sp, a_flag->s, a_flag->polarity);
                }
            }
        } else {
            log_debug("%*sflags: none", fl_indent, fl_sp);
        }
    }
}

EXPORT void dump_property(int fl_indent, struct obzl_meta_property *prop)
{
    /* log_trace("dump_property %p", prop); */
    log_debug("%*sdump_property:", fl_indent, fl_sp);
    log_debug("%*sname: " UGRN "%s" "\033[0m",
              fl_delta+fl_indent, fl_sp, prop->name);
    dump_settings(fl_delta+fl_indent, prop->settings);
}

EXPORT void dump_properties(int fl_indent, UT_array *props)
{
    /* log_trace("dump_properties: %p", props); */
    struct obzl_meta_property *p = NULL;
    while ( (p=(struct obzl_meta_property *)utarray_next(props, p))) {
        dump_property(fl_delta+fl_indent, p);
    }
}

EXPORT void dump_setting(int fl_indent, struct obzl_meta_setting *setting)
{
    log_trace("%*ssetting:", fl_indent, fl_sp);
    dump_flags(2*fl_delta+fl_indent, setting->flags);
    log_debug("%*sopcode: %s", fl_delta+fl_indent, fl_sp,
              setting->opcode == 0 ? "SET" : "UPDATE");
    dump_values(2*fl_delta+fl_indent, setting->values);
    /* log_trace("%*sdump_setting() finished", fl_indent, fl_sp); */
}

EXPORT void dump_settings(int fl_indent, obzl_meta_settings *settings)
{
    log_trace("%*ssettings:", fl_indent, fl_sp);
    obzl_meta_setting *setting = NULL;
    for(setting  = utarray_front(settings->list);
        setting != NULL;
        setting  = utarray_next(settings->list, setting)) {
        dump_setting(fl_delta+fl_indent, setting);
    }
}

void dump_values(int fl_indent, obzl_meta_values *values)
{
    /* log_trace("dump_values %p", values); */
    fl_indent++;            /* account for width of log label */
    if ( values->list ) {
        if (utarray_len(values->list) == 0) {
            log_debug("%*svalues: none", fl_indent, fl_sp);
        } else {
            char **a_value = NULL;
            log_trace("%*svalues:", fl_indent, fl_sp);
            while ( (a_value=(char **)utarray_next(values->list, a_value))) {
                log_trace("%*s" GRN "%s" CRESET,
                          fl_delta+fl_indent, fl_sp,
                          *a_value);
            }
        }
    } else {
        log_debug("%*svalues: none", fl_indent, fl_sp);
    }
}
