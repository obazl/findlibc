#if INTERFACE
#include <stdbool.h>
#include "utarray.h"
#endif

#include <stdio.h>
#include "uthash.h"

#include "liblogc.h"

#include "meta_entries.h"

#if defined(PROFILE_fastbuild)
#define DEBUG_LEVEL findlibc_debug
extern int  DEBUG_LEVEL;
#define TRACE_FLAG findlibc_trace
extern bool TRACE_FLAG;
#endif

/* **************************************************************** */
#if EXPORT_INTERFACE
enum obzl_meta_entry_type_e { OMP_PROPERTY, OMP_PACKAGE };
struct obzl_meta_entry {
    enum obzl_meta_entry_type_e type;
    union {
        struct obzl_meta_property *property;
        struct obzl_meta_package  *package;
    };
};

struct obzl_meta_entries {
    UT_array *list;          /* list of obzl_meta_entry */
};
#endif

EXPORT UT_icd entry_icd = {sizeof(struct obzl_meta_entry), NULL, entry_copy, entry_dtor};

/* **************************************************************** */
EXPORT int obzl_meta_entries_count(obzl_meta_entries *_entries)
{
    return utarray_len(_entries->list);
}

/* **************** */
EXPORT obzl_meta_entry *obzl_meta_entries_nth(obzl_meta_entries *_entries, unsigned int _i)
{
    /* if (_i < 0) { */
    /*     LOG_ERROR(0, "Index < min (1)"); */
    /* } else { */
        if (_i > utarray_len(_entries->list)) {
            LOG_ERROR(0, "Index > max (%d)", utarray_len(_entries->list));
        } else {
            return utarray_eltptr(_entries->list, _i);
        }
    /* } */
    /* FIXME: set errno, return NULL? */
    exit(EXIT_FAILURE);
}

/* **************** */
EXPORT obzl_meta_property *obzl_meta_entries_property(obzl_meta_entries *_entries, char *_name)
{
    TRACE_ENTRY_MSG("prop: %s", _name);

    /* utarray_find requires a sort; not worth the cost */
    obzl_meta_entry *e = NULL;
    for (int i = 0; i < obzl_meta_entries_count(_entries); i++) {
        e = obzl_meta_entries_nth(_entries, i);
        if (e->type == OMP_PROPERTY) {
            /* LOG_DEBUG(0, "property: %s := %s", */
            /*           e->property->name, */
            /*           obzl_meta_property_value(e->property)); */
            if (strncmp(e->property->name, _name, 256) == 0) {
                return e->property;
            }
        }
        /* LOG_DEBUG(0, "iteration %d", i); */
    }
    return NULL;
}

/* **************** */
EXPORT char *obzl_meta_directory_property(obzl_meta_entries *_entries)
{
    TRACE_ENTRY;

    if (_entries == NULL) return NULL;
    /* utarray_find requires a sort; not worth the cost */
    obzl_meta_entry *e = NULL;
    for (int i = 0; i < obzl_meta_entries_count(_entries); i++) {
        e = obzl_meta_entries_nth(_entries, i);
        if (e->type == OMP_PROPERTY) {
            /* LOG_DEBUG(0, "property: %s := %s", */
            /*           e->property->name, */
            /*           obzl_meta_property_value(e->property)); */
            if (strncmp(e->property->name, "directory", 9) == 0) {
                return obzl_meta_property_value(e->property);
            }
        }
        /* LOG_DEBUG(0, "iteration %d", i); */
    }
    return NULL;
}

/* **************** */
void entry_copy(void *_dst, const void *_src) {
    struct obzl_meta_entry *dst = (struct obzl_meta_entry*)_dst;
    struct obzl_meta_entry *src = (struct obzl_meta_entry*)_src;
    dst->type = src->type;
    dst->property = src->property;
    dst->package = src->package;
}

void entry_dtor(void *_elt) {
    (void)_elt;
    /* struct obzl_meta_entry *elt = (struct obzl_meta_entry*)_elt; */
    /* if (elt->s) free(elt->s); */
}

/* **************************************************************** */
void normalize_entries(obzl_meta_entries *entries, obzl_meta_entry *_entry)
{
    TRACE_ENTRY;
#if DEBUG_ENTRIES
    /* LOG_TRACE(0, "normalize_entries()"); */
    /* if (_entry->type == OMP_PROPERTY) { */
    /*     LOG_TRACE(0, "new entry type: property"); */
    /*     LOG_TRACE(0, "\tname: %s", _entry->property->name); */
    /* } else { */
    /*     LOG_TRACE(0, "new entry type: package"); */
    /* } */
#endif

    int ct = obzl_meta_entries_count(entries);
    LOG_DEBUG(0, "entries ct: %d", ct);
    obzl_meta_entry *e = NULL;

    bool matched = false;
    for (int i = 0; i < ct; i++) {
        e = obzl_meta_entries_nth(entries, i);

        if (e->type == _entry->type) {
            /* LOG_TRACE(0, "old entry type: property"); */
            /* LOG_TRACE(0, "\tname: %s", e->property->name); */
            if (e->type == OMP_PROPERTY) {
                if (strncmp(e->property->name, _entry->property->name, 32) == 0) {
                    /* LOG_TRACE(0, "match"); */
                    matched = true;
                    /* update the entry  */
                    /* obzl_meta_settings *settings = e->property->settings; */
                    /* obzl_meta_settings *new_settings = _entry->property->settings; */
                    utarray_concat(e->property->settings->list, _entry->property->settings->list);
                }
            }
        /* } else { */
            /* LOG_TRACE(0, "old entry type: package"); */
        }
    }
    if ( !matched ) {
        utarray_push_back(entries->list, _entry);
    }
}

/* **************************************************************** */
EXPORT obzl_meta_entry *obzl_meta_entry_new(void)
{
    return (obzl_meta_entry*)calloc(sizeof(obzl_meta_entry), 1);
}

EXPORT enum obzl_meta_entry_type_e obzl_meta_entry_type(obzl_meta_entry *e)
{
    return e->type;
}

EXPORT obzl_meta_property *obzl_meta_entry_property(obzl_meta_entry *e)
{
    return e->property;
}

EXPORT obzl_meta_package *obzl_meta_entry_package(obzl_meta_entry *e)
{
    return e->package;
}

