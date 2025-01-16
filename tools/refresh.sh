#!/bin/sh

bazel --bazelrc=/dev/null build lib/lexer/lexis:meta_lexis.c --//lib/lexer:dev
cp -vf .bazel/bin/lib/lexer/lexis/meta_lexis.c lib/lexer/

bazel --bazelrc=/dev/null build lib/parser/syntaxis:meta_syntaxis.c --//lib/parser:dev
cp -vf .bazel/bin/lib/parser/syntaxis/meta_syntaxis.c lib/parser/

bazel --bazelrc=/dev/null build lib/parser:mkhdrs --//lib/parser:dev
cp -vf .bazel/bin/lib/parser/syntaxis/meta_syntaxis.h lib/parser/


# bazel build lib/lexer:mkhdrs
# cp .bazel/bin/lib/lexer/lexis/opam_lexis.h lib/lexer/
