#include "log.h"
#include "utarray.h"

#include "debug_meta.h"

int indent = 2;
int delta = 2;
char *sp = " ";

bool debug_meta;

#if EXPORT_INTERFACE
#define DUMP_PKG(indent, pkg) \
    log_debug("DUMPING PKG"); \
    dump_package(indent, pkg)
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

void dump_entry(int indent, struct obzl_meta_entry *entry)
{
    log_trace("%*sdump_entry:", indent, sp);
    log_debug("%*sentry type: %d", delta+indent, sp, entry->type);
    if (entry->type == OMP_PROPERTY) {
        dump_property(delta+indent, entry->property);
    } else {
        dump_package(delta+indent, entry->package);
    }
}

void dump_entries(int indent, struct obzl_meta_entries *entries)
{
    log_trace("%*sdump_entries() %p", indent, sp, entries);
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

void dump_property(int indent, struct obzl_meta_property *prop)
{
    /* log_trace("dump_property %p", prop); */
    log_debug("%*sproperty:", indent, sp);
    log_debug("%*sname: %s", delta+indent, sp, prop->name);
    dump_settings(delta+indent, prop->settings);
}

void dump_properties(int indent, UT_array *props)
{
    /* log_trace("dump_properties: %p", props); */
    struct obzl_meta_property *p = NULL;
    while ( (p=(struct obzl_meta_property *)utarray_next(props, p))) {
        dump_property(delta+indent, p);
    }
}
