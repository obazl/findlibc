#include "log.h"

#define META_TRACE_ENTRY log_trace(RED "ENTRY:" CRESET " %s", __func__);

#define TRACE_ENTRY if (findlib_trace) log_trace(RED "ENTRY:" CRESET " %s", __func__);

#define TRACE_LOG(fmt, ...) if (findlib_trace) log_trace(fmt, __VA_ARGS__)

#define TRACE_EXIT if (findlib_trace) log_trace(RED "EXIT:" CRESET " %s", __func__);

#define TRACE_ENTRY_MSG(fmt, ...) \
    if (findlib_trace) log_trace(RED "ENTRY:" CRESET " %s, " fmt, __func__, __VA_ARGS__);

#define TRACE_S7_DUMP(msg, x) (({char*s=s7_object_to_c_string(s7, x);log_debug("%s: '%.60s' (first 60 chars)", msg, s);fflush(NULL);free(s);}))
#define DUMP_PREFIX(pfx) \
    if (pfx) {                                          \
        log_debug("\tpfx start: '%.15s'", (pfx)->start);     \
        log_debug("\tpfx len: '%d'", (pfx)->len);     \
    } else {                                            \
        log_debug("prefix: NULL");                      \
    }

#define LOG_DEBUG(lvl, fmt, ...) if (findlib_debug>lvl) log_debug(fmt, __VA_ARGS__)
#define LOG_ERROR(lvl, fmt, ...) if (findlib_debug>lvl) log_error(fmt, __VA_ARGS__)
#define LOG_INFO(lvl, fmt, ...)  if (findlib_debug>lvl) log_info(fmt, __VA_ARGS__)
#define LOG_TRACE(lvl, fmt, ...) if (findlib_debug>lvl) log_trace(fmt, __VA_ARGS__)
#define LOG_WARN(lvl, fmt, ...)  if (findlib_debug>lvl) log_warn(fmt, __VA_ARGS__)
