#include "log.h"
#include "utarray.h"

#include "meta_debugging.h"

int indent = 2;
int delta = 2;
char *sp = " ";

bool debug_meta;

#if EXPORT_INTERFACE
#if defined(DEVBUILD)
#define DUMP_PKG(indent, pkg) \
    log_debug("FINDLIB PKG: %s", pkg->name); \
    dump_package(indent, pkg)
#else
#define DUMP_PKG(indent, pkg)
#endif
#endif

EXPORT void dump_package(int indent, struct obzl_meta_package *pkg)
{
    log_debug("%*sdump_package:", indent, sp);
    log_debug("%*sname:      %s", delta+indent, sp, pkg->name);
    log_debug("%*sdirectory: %s", delta+indent, sp, pkg->directory);
    log_debug("%*spath: %s", delta+indent, sp, pkg->path);
    log_debug("%*smetafile:  %s", delta+indent, sp, pkg->metafile);
    dump_entries(delta+indent, pkg->entries);
}

EXPORT void dump_entry(int indent, struct obzl_meta_entry *entry)
{
    log_trace("%*sdump_entry:", indent, sp);
    log_debug("%*sentry type: %s", delta+indent, sp,
              entry->type == OMP_PROPERTY
              ? "property"
              : "package");
    if (entry->type == OMP_PROPERTY) {
        dump_property(delta+indent, entry->property);
    } else {
        dump_package(delta+indent, entry->package);
    }
}

EXPORT void dump_entries(int indent, struct obzl_meta_entries *entries)
{
    log_trace("%*sdump_entries()", indent, sp); //, entries);
    if (entries == NULL) {
        log_trace("%*sentries: none", indent, sp);
    } else {
        obzl_meta_entry *e = NULL;
        for (int i = 0; i < obzl_meta_entries_count(entries); i++) {
            e = obzl_meta_entries_nth(entries, i);
            /* log_trace("e: %p", e); */
            /* log_trace("e type: %d", e->type); */
            dump_entry(delta+indent, e);
        }
        /* log_trace("%*sdump_entries() DONE", indent, sp); */
    }
}

EXPORT void dump_flags(int indent, obzl_meta_flags *flags)
{
    indent++;            /* account for width of log label */
    /* log_trace("%*sdump_flags", indent, sp); */
    if (flags == NULL) {
        log_trace("%*sflags: none", indent, sp);
        return;
    } else {
        if ( flags->list ) {
            if (utarray_len(flags->list) == 0) {
                log_trace("%*sflags: none.", indent, sp);
                return;
            } else {
                log_trace("%*sflags ct: %d", indent, sp, utarray_len(flags->list));
                struct obzl_meta_flag *a_flag = NULL;
                while ( (a_flag=(struct obzl_meta_flag*)utarray_next(flags->list, a_flag))) {
                    log_debug("%*s%s (polarity: %d)", delta+indent, sp, a_flag->s, a_flag->polarity);
                }
            }
        } else {
            log_debug("%*sflags: none", indent, sp);
        }
    }
}

EXPORT void dump_property(int indent, struct obzl_meta_property *prop)
{
    /* log_trace("dump_property %p", prop); */
    log_debug("%*sdump_property:", indent, sp);
    log_debug("%*sname: " UGRN "%s" "\033[0m",
              delta+indent, sp, prop->name);
    dump_settings(delta+indent, prop->settings);
}

EXPORT void dump_properties(int indent, UT_array *props)
{
    /* log_trace("dump_properties: %p", props); */
    struct obzl_meta_property *p = NULL;
    while ( (p=(struct obzl_meta_property *)utarray_next(props, p))) {
        dump_property(delta+indent, p);
    }
}

EXPORT void dump_setting(int indent, struct obzl_meta_setting *setting)
{
    log_trace("%*ssetting:", indent, sp);
    dump_flags(2*delta+indent, setting->flags);
    log_debug("%*sopcode: %s", delta+indent, sp,
              setting->opcode == 0 ? "SET" : "UPDATE");
    dump_values(2*delta+indent, setting->values);
    /* log_trace("%*sdump_setting() finished", indent, sp); */
}

EXPORT void dump_settings(int indent, obzl_meta_settings *settings)
{
    log_trace("%*ssettings:", indent, sp);
    obzl_meta_setting *setting = NULL;
    for(setting  = utarray_front(settings->list);
        setting != NULL;
        setting  = utarray_next(settings->list, setting)) {
        dump_setting(delta+indent, setting);
    }
}

void dump_values(int indent, obzl_meta_values *values)
{
    /* log_trace("dump_values %p", values); */
    indent++;            /* account for width of log label */
    if ( values->list ) {
        if (utarray_len(values->list) == 0) {
            log_debug("%*svalues: none", indent, sp);
        } else {
            char **a_value = NULL;
            log_trace("%*svalues:", indent, sp);
            while ( (a_value=(char **)utarray_next(values->list, a_value))) {
                log_trace("%*s" GRN "%s" CRESET,
                          delta+indent, sp,
                          *a_value);
            }
        }
    } else {
        log_debug("%*svalues: none", indent, sp);
    }
}
