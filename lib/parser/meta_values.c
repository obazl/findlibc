#include <stdbool.h>
#include <stdio.h>

#include "utarray.h"
#include "uthash.h"
#include "liblogc.h"

#include "meta_values.h"

extern const UT_icd ut_str_icd;

#define DEBUG_LEVEL debug_findlibc
extern int  DEBUG_LEVEL;
#define TRACE_FLAG trace_findlibc
extern bool TRACE_FLAG;

#if defined(DEBUG_VALUES) || defined (TRACING)
/* extern int indent; */
/* extern int delta; */
/* extern char *sp; */
#endif

#if EXPORT_INTERFACE
typedef char *obzl_meta_value;

struct obzl_meta_values {
    UT_array *list;             /* list of strings  */
};
#endif

/* **************************************************************** */
EXPORT int obzl_meta_values_count(obzl_meta_values *_values)
{
    return utarray_len(_values->list);
}

EXPORT obzl_meta_value *obzl_meta_values_nth(obzl_meta_values *_values, unsigned int _i)
{
    return utarray_eltptr(_values->list, _i);
}

EXPORT obzl_meta_values *obzl_meta_values_new(char *valstr)
{
    TRACE_ENTRY
#if DEBUG_VALUES
    /* log_trace("obzl_meta_values_new(%s)", valstr); */
#endif
    obzl_meta_values *new_values = (obzl_meta_values*)calloc(sizeof(obzl_meta_values),1);
    utarray_new(new_values->list, &ut_str_icd);
    if (valstr != NULL)
        utarray_push_back(new_values->list, &valstr);
    /* dump_values(0, new_values); */
    TRACE_EXIT
    return new_values;
}

EXPORT obzl_meta_values *obzl_meta_values_new_copy(obzl_meta_values *values)
{
/* #if DEBUG_VALUES */
/*     log_trace("obzl_meta_values_new_copy"); */
/* #endif */
    obzl_meta_values *new_values = (obzl_meta_values*)calloc(sizeof(obzl_meta_values),1);
    utarray_new(new_values->list, &ut_str_icd);
    utarray_concat(new_values->list, values->list);
    /* dump_values(0, new_values); */
    return new_values;
}

/* EXPORT UT_array *obzl_meta_values_new_tokenized(char *valstr) */
EXPORT obzl_meta_values *obzl_meta_values_new_tokenized(char *valstr)
{
#if DEBUG_VALUES
    log_trace("obzl_meta_values_new_tokenized");
#endif
    /* UT_array *new_values; */
    obzl_meta_values *new_values = (obzl_meta_values*)calloc(sizeof(obzl_meta_values), 1);
    utarray_new(new_values->list, &ut_str_icd);
    char *token, *sep = " ,\n";
    token = strtok(valstr, sep);
    while( token != NULL ) {
#if DEBUG_VALUES
        log_trace("pushing val %s", token);
#endif
        utarray_push_back(new_values->list, &token);
        token = strtok(NULL, sep);
    }
    return new_values;
}
