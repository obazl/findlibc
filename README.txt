Need to make a distinction between:

* findlib (META files)
* OPAM, which uses/organizes META files.

Strictly speaking libfindlib should have narrow scope of just reading
META files. Navigating an OPAM switch is a separate issue.

HOWEVER: META files may include cross-refs to other META files. IOW
findlib assumes a certain fs structure. But these x-refs are always to
subdirs? No, they can refer to the stdlib location. Which is
determined by the OCaml compiler build config, but may be set in
findlib.conf.

WHAT about findlib.conf and site-lib?


functional units:

opam_config - discovers current opam switch, dir paths, etc.

opam switch crawler - walks an opam switch finding META files

libfindlib: just reads META files

emitters - call into libfindlib API
