#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>

#include "log.h"

#include "meta_parser.h"

struct obzl_meta_package *MAIN_PKG;

LOCAL bool _is_empty(const char *s)
{
  while (*s) {
    if (!isspace(*s))
      return false;
    s++;
  }
  return true;
}

char *package_name_from_file_name(char *fname)
{
    char *bn = basename(fname);
    int x = strlen(bn) - 5;
    if ( strncmp(&bn[x], ".META", 5) == 0) {
        bn[x] = '\0';
        return bn;
    } else {
        return basename(dirname(fname));
    }
}

EXPORT struct obzl_meta_package *obzl_meta_parse_file(char *_fname)
{
#if defined(TRACING)
/* #ifdef DEBUG */
/*     log_set_quiet(false); */
/* #else */
/*     log_set_quiet(true); */
/* #endif */
    /* log_trace("obzl_meta_parse_file: %s", _fname); */
#endif

    /* we're using dirname which can mutate its arg. we need to return fname as-is */
    errno = 0;
    char *fname = strdup(_fname);

    /* log_info("strduped %s\n", fname); */

    FILE *f;

    errno = 0;
    f = fopen(fname, "r");
    if (f == NULL) {
        /* errnum = errno; */
        log_error("fopen failure for %s", fname);
        log_error("Value of errno: %d", errno);
        log_error("fopen error %s", strerror( errno ));
        return NULL;
    }
#if defined(TRACING)
    /* log_debug("fopened %s", fname); */
#endif
    fseek(f, 0, SEEK_END);
    const size_t fsize = (size_t) ftell(f);
    if (fsize == 0) {
#if defined(TRACING)
        log_debug("%s fsize == 0; returning NULL", fname);
#endif
        fclose(f);
        errno = -1;
        return NULL;
    }
    fseek(f, 0, SEEK_SET);
    char *buffer = (char*) malloc(fsize + 1);
    size_t read_ct = fread(buffer, 1, fsize, f);
    (void)read_ct;              /* prevent -Wunused-variable */
#if defined(TRACING)
    /* log_info("readed: %d", read_ct); */
#endif
    buffer[fsize] = 0;
    fclose(f);

    if (_is_empty(buffer)) {
        fclose(f);
        errno = -2;
        return NULL;
    }

#if defined(TRACING)
    /* log_debug("lexing"); */
#endif
    /* THE_METAFILE[0] = '\0'; */
    /* mystrcat(THE_METAFILE, fname); */

    struct meta_lexer_s *meta_lexer = malloc(sizeof(struct meta_lexer_s));
    meta_lexer_init(meta_lexer, fname, buffer);

    void* pMetaParser = ParseAlloc (malloc);
    /* InitParserState(ast); */
    /* ParseTrace(stdout, "trace_"); */
    int tok;
    union meta_token *mtok = malloc(sizeof(union meta_token));

    /* if (logger.lex_verbosity == 0) */
    /*     log_set_quiet(true); */
    /* else */
    /*     log_set_quiet(logger.quiet); */
    /* log_set_level(logger.lex_log_level); */

    /* log_set_quiet(false); */
    /* log_set_level(LOG_TRACE); */
    /* log_info("starting"); */
    /* log_set_quiet(true); */

    MAIN_PKG = (struct obzl_meta_package*)calloc(sizeof(struct obzl_meta_package), 1);
    MAIN_PKG->name      = package_name_from_file_name(strdup(fname));
    MAIN_PKG->path      = dirname(strdup(fname));
    MAIN_PKG->directory = MAIN_PKG->name; // dirname(fname);
    MAIN_PKG->metafile  = fname;

    while ( (tok = get_next_meta_token(meta_lexer, mtok)) != 0 ) {
        /* log_set_quiet(true); */
#if defined(DEBUG_LEX)
        switch(tok) {
        case DIRECTORY:
            log_trace("lex DIRECTORY: %s", mtok->s); break;
        case FLAGS:
            log_trace("lex FLAGS: %s", mtok->s); break;
        case VNAME:
            log_trace("lex VNAME: %s", mtok->s); break;
        case WORD:
            log_trace("lex WORD: %s", mtok->s); break;
        case WORDS:
            log_trace("lex WORDS: %s", mtok->s); break;
        case DQ:
            log_trace("DQ"); break;
        case EQ:
            log_trace("lex EQ"); break;
        case PLUSEQ:
            log_trace("lex PLUSEQ"); break;
        case LPAREN:
            log_trace("lex LPAREN"); break;
        case RPAREN:
            log_trace("lex RPAREN"); break;
        case VERSION:
            log_trace("lex VERSION: %s", mtok->s);
            break;
        case DESCRIPTION:
            log_trace("lex DESCRIPTION: %s", mtok->s);
            break;
        case REQUIRES:
            log_trace("lex REQUIRES"); break;
        case PACKAGE:
            log_trace("lex PACKAGE: %s", mtok->s); break;
        case WARNING:
            log_trace("WARNING"); break;
        case ERROR:
            log_trace("ERROR"); break;
        default:
            log_trace("other: %d", tok); break;
        }
#endif
        Parse(pMetaParser, tok, mtok, MAIN_PKG); // , &sState);

        mtok = malloc(sizeof(union meta_token));
        /* if (logger.lex_verbosity == 0) */
        /*     log_set_quiet(false); */
        /* else */
        /*     log_set_quiet(logger.quiet); */
        /*     log_set_level(logger.lex_log_level); */
    }

    /*     if (logger.parse_verbosity == 0) */
    /*         log_set_quiet(false); */
    /*     else */
    /*         log_set_quiet(logger.quiet); */
    /*         log_set_level(logger.parse_log_level); */
    /* log_set_quiet(true); */

    /* log_trace("lex: end of input"); */

    Parse(pMetaParser, 0, mtok, MAIN_PKG); // , &sState);
    ParseFree(pMetaParser, free );

    /* if (logger.verbosity == 0) */
    /*     log_set_quiet(false); */
    /* else */
    /*     log_set_quiet(logger.quiet); */
    /* log_set_level(logger.log_level); */

    /* log_set_quiet(false); */

    /* log_trace("PARSED %s", fname); */

    free(buffer);
    return MAIN_PKG;
}
