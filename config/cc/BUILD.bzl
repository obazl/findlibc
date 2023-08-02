load("//:BUILD.bzl", "LIBLOG_CC_VERSION")

BASE_SRCS = ["//config:config.h", "//config:ansi_colors.h"]

BASE_DEPS = [ ## only vendored
    # "//vendored/CException",
    "@liblog_cc//src:logc",
]

BASE_INCLUDE_PATHS = [
    "-Iconfig",
    "-Iexternal/mibl/config",
    # "-Ivendored/CException",
    # "-Iexternal/mibl/vendored/CException",
    "-Iexternal/liblog_cc~{}/src".format(LIBLOG_CC_VERSION)
]

BASE_COPTS = [
    "-x", "c",
    "-Wall",
    "-Wextra",
    # GCC:
    "-Werror", # turn all warnings into errors
    "-Wfatal-errors", # stop on first error
] + select({
    "//config/host/build:macos?": [
        "-std=c11",
        "-Werror=pedantic", # not needed with -Werror?
        "-Wpedantic", # same as -pedantic, strict ISO C and ISO C++ warnings
        "-pedantic-errors",
        "-Wno-gnu-statement-expression",
        # "-Werror=pedantic",
        # "-Wno-gnu",
        # "-Wno-format-pedantic",
    ],
    "//config/host/build:linux?": [
        "-std=gnu11",
        "-fPIC",
        # "-Wl,--no-undefined",
    ],
    "//conditions:default": ["-std=c11"],
})

BASE_LINKOPTS = select({
    "//config/host/build:linux?": ["-rdynamic", "-ldl"],
    "//config/host/build:macos?": [], ## "-ldl"],
    "//conditions:default": []
})

BASE_DEFINES = select({
    "//config/host/build:macos?": ["DSO_EXT=\\\".dylib\\\""],
    "//config/host/build:linux?": [
        "DSO_EXT=\\\".so\\\"",
        # "_XOPEN_SOURCE=500", # strdup
        "_POSIX_C_SOURCE=200809L", # strdup, strndup since glibc 2.10
        "_DEFAULT_SOURCE",    # dirent macros
        "_GNU_SOURCE"         # dlsym RTLD macros
    ],
    "//conditions:default":   ["DSO_EXT=\\\".so\\\""]
}) + select({
    "//config/profile:dev?": ["DEVBUILD"],
    "//conditions:default": []
}) + select({
    "//config/trace:tracing?": ["TRACING"],
    "//conditions:default": []
})

def pkg_include(pkg):
    if native.repository_name() == "@":
        return pkg # native.package_name()
    else:
        return "external/{}~{}/{}".format(
            native.module_name(), native.module_version(), pkg) # native.package_name())
