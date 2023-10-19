#include <ctype.h>
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

#include <unistd.h>

#include "liblogc.h"

#include "utarray.h"
#include "utstring.h"
#include "utstring.h"

#include "lex_test.h"

#include "liblogc.h"

#if defined(PROFILE_fastbuild)
#define DEBUG_LEVEL findlibc_debug
extern int  DEBUG_LEVEL;
#define TRACE_FLAG findlibc_trace
extern bool TRACE_FLAG;
#endif

LOCAL bool _is_empty(const char *s)
{
  while (*s) {
    if (!isspace(*s))
      return false;
    s++;
  }
  return true;
}

LOCAL void lex_META_file(char *_fname)
{
#if defined(TRACING)
/* #ifdef DEBUG */
/*     log_set_quiet(false); */
/* #else */
/*     log_set_quiet(true); */
/* #endif */

#endif
    log_debug("CWD: %s", getcwd(NULL,0));
    log_trace("lex_META_file: %s", _fname);

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
        exit(EXIT_FAILURE);
    }
#if defined(TRACING)
    log_debug("fopened %s", fname);
#endif
    fseek(f, 0, SEEK_END);
    const size_t fsize = (size_t) ftell(f);
    if (fsize == 0) {
#if defined(TRACING)
        log_debug("%s fsize == 0; returning NULL", fname);
#endif
        fclose(f);
        errno = -1;
        /* return NULL; */
        exit(EXIT_FAILURE);
    }
    fseek(f, 0, SEEK_SET);
    char *buffer = (char*) malloc(fsize + 1);
    size_t read_ct = fread(buffer, 1, fsize, f);
    (void)read_ct;              /* prevent -Wunused-variable */
#if defined(TRACING)
    log_info("readed: %d", read_ct);
#endif
    buffer[fsize] = 0;
    fclose(f);

    if (_is_empty(buffer)) {
        fclose(f);
        errno = -2;
        /* return NULL; */
        exit(EXIT_FAILURE);
    }

#if defined(TRACING)
    log_debug("lexing");
#endif

    struct meta_lexer_s *meta_lexer = malloc(sizeof(struct meta_lexer_s));
    meta_lexer_init(meta_lexer, fname, buffer);

    int tok;
    // currently meta_token is struct{char *s}
    union meta_token *mtok = malloc(sizeof(union meta_token));

    /* if (logger.lex_verbosity == 0) */
    /*     log_set_quiet(true); */
    /* else */
    /*     log_set_quiet(logger.quiet); */
    /* log_set_level(logger.lex_log_level); */

    /* log_set_quiet(false); */
    /* log_set_level(LOG_TRACE); */
    log_info("starting");
    /* log_set_quiet(true); */

    while ( (tok = get_next_meta_token(meta_lexer, mtok)) != 0 ) {
        /* log_set_quiet(true); */
/* #if defined(DEBUG_LEX) */
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
        /* if (mtok->s) free(mtok->s); */
/* #endif */
        /* Parse(pMetaParser, tok, mtok, MAIN_PKG); // , &sState); */

        /* mtok = malloc(sizeof(union meta_token)); */

        /* if (logger.lex_verbosity == 0) */
        /*     log_set_quiet(false); */
        /* else */
        /*     log_set_quiet(logger.quiet); */
        /*     log_set_level(logger.lex_log_level); */
    }

    log_trace("lex: end of input");

    free(buffer);
}

int main(int argc, char *argv[])
{
    int opt;

    UT_string *findlib_file;
    utstring_new(findlib_file);

    while ((opt = getopt(argc, argv, "dtf:hv")) != -1) {
        switch (opt) {
#if defined(PROFILE_fastbuild)
        case 'd':
            findlibc_debug++;
            break;
        case 't':
            findlibc_trace = true;
            break;
#endif
        case 'f':
            /* BUILD.bazel or BUILD file */
            utstring_printf(findlib_file, "%s", optarg);
            break;
        case 'h':
            log_info("Help: ");
            exit(EXIT_SUCCESS);
        default:
            log_error("Usage: %s [-f] [findlibfile]", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (utstring_len(findlib_file) == 0) {
        log_error("-f <findlibfile> must be provided.");
        exit(EXIT_FAILURE);
    }

    char *wd = getenv("BUILD_WORKING_DIRECTORY");
    if (wd) {
        /* we launched from bazel workspace, cd to launch dir */
        chdir(wd);
    }

    lex_META_file(utstring_body(findlib_file));
    /* UT_array *result = opam_lex_file(utstring_body(opam_file)); */

    /* UT_array *result = sealark_lex_string("'hello'\n#cmt1\n"); */

    /* log_debug("main RESULT dump:"); */
    /* dump_nodes(result); */
}
