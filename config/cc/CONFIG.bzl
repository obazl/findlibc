load("@cc_config//:CONFIG.bzl",
     _BASE_COPTS    = "BASE_COPTS",
     _BASE_LINKOPTS = "BASE_LINKOPTS")

BASE_SRCS          = []
BASE_DEPS          = ["//src:findlibc_debug"]
BASE_INCLUDE_PATHS = []
BASE_COPTS         = _BASE_COPTS
BASE_LINKOPTS      = _BASE_LINKOPTS
BASE_DEFINES       = ["DEBUG_$(COMPILATION_MODE)"]

