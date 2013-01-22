#! /usr/bin/env python

import os
import sys

BOOST_VERSION = "1.49"
EnsureSConsVersion(2, 1)

# checks boost version
def CheckBoost(context, version):
    # boost versions are in format major.minor.subminor
    v_arr = version.split(".")
    version_n = 0
    if len(v_arr) > 0:
        version_n += int(v_arr[0])*100000
    if len(v_arr) > 1:
        version_n += int(v_arr[1])*100
    if len(v_arr) > 2:
        version_n += int(v_arr[2])

    context.Message("Checking for Boost version >= %s..." % version)
    ret = context.TryCompile(
"""
#include <boost/version.hpp>

#if BOOST_VERSION < %d
#error Installed boost is too old!
#endif
int main()
{
    return 0;
}

""" % version_n, ".cc")
    context.Result(ret)
    return ret

### BASE DEFS ###
USE_SSE = "USE_SSE"
USE_OPENSSL = "USE_OPENSSL"
WITH_CRYPTO = "WITH_CRYPTO"
WITH_UNIT_TESTS = "WITH_UNIT_TESTS"
WITH_PROFILE = "WITH_PROFILE"

### BASE DIRECTORIES ###
src_name = "snippets"
src_dir = "src"
base_build_dir = "build"
build_dir = base_build_dir
lib_dir = "lib"
bin_dir = "bin"
conf_dir = os.path.join("etc", src_name)
data_dir = os.path.join("share", src_name)

### BASE COMPILER OPTIONS ###
ccflags = [
    "-std=c++11",
    "-msse3",
    "-pedantic-errors",
    "-Wall",
    "-Wextra",
    "-Weffc++",
    "-Woverloaded-virtual",
    "-Wformat=2",
#    "-Wold-style-cast",
#    "-Wshadow",
    "-pipe",
    "-fstack-protector",        # TODO: this is probably redundant to -fstack-protector-all
    "-fstack-protector-all",
    "-fstack-check",

    # these warnings aren't too critical
    # and can be very spammy (thanks boost!)
    "-Wno-unused-parameter",
    "-Wno-unknown-pragmas",
    "-Wno-non-virtual-dtor",    # TODO: why is this ignored?
]
ccpath = [
    os.getcwd(),
    "/opt/local/include",       # osx darwin ports
]
ccdefs = [ USE_SSE, USE_OPENSSL, WITH_CRYPTO, "_FORTIFY_SOURCE=2" ]
ldflags = []
ldpath = [
    os.path.join(os.getcwd(), lib_dir),
    "/opt/local/lib",           # osx darwin ports
]
libs = []

### TARGETS ###
core_lib = "%s-core" % src_name
engine_lib = "%s-engine" % src_name

tests_app = src_name

### META TARGETS ###
build_meta_targets = [ "unittest" ]

if int(ARGUMENTS.get("ctags", 0)):
    build_meta_targets.append("ctags")
elif int(ARGUMENTS.get("coreonly", 0)):
    pass
elif int(ARGUMENTS.get("testcheckin", 0)):
    build_meta_targets.append("unittest")
    build_meta_targets.append("runtests")
else:
    if int(ARGUMENTS.get("unittest", 0)):
        build_meta_targets.append("unittest")

### ARGUMENTS ###
install_dir = ARGUMENTS.get("installdir", "")
print("Using install dir: %s" % install_dir)
ccdefs.append("INSTALLDIR=\\\"%s\\\"" % install_dir)
ccdefs.append("BINDIR=\\\"%s\\\"" % bin_dir)
ccdefs.append("CONFDIR=\\\"%s\\\"" % conf_dir)
ccdefs.append("DATADIR=\\\"%s\\\"" % data_dir)

if "unittest" in build_meta_targets:
    ccdefs.append(WITH_UNIT_TESTS)
    ccflags.extend([ "--coverage" ]) #ccflags.extend([ "-fprofile-arcs", "-ftest-coverage" ])
    ldflags.append("-ldl")
    ldflags.append("--coverage") #libs.append("gcov")
    build_dir = os.path.join(build_dir, "test")

    core_lib += "-test"
    engine_lib += "-test"

profile = int(ARGUMENTS.get("profile", 0))
if profile:
    ccflags.extend([ "-pg" ])
    ccdefs.append(WITH_PROFILE)
    ldflags.append("-pg")
    build_dir = os.path.join(build_dir, "profile")

    core_lib += "-profile"
    engine_lib += "-profile"

    tests_app += "-profile"

efence = int(ARGUMENTS.get("efence", 0))

if int(ARGUMENTS.get("release", 0)):
    ccflags.extend([
        "-O3",
        "-ffast-math",
        "-fno-unsafe-math-optimizations",
        "-fno-common",
        "-funroll-loops",
        #"-fomit-frame-pointer",
        #"-Winline",
    ])
    #ccdefs.append("_NDEBUG")
    ccdefs.append("NDEBUG")
    build_dir = os.path.join(build_dir, "release")
else:
    ccflags.extend([
        "-O0",
        "-g",
        "-fno-default-inline",
    ])

    if sys.platform == "darwin":
        ccflags.extend([
            "-gdwarf-2",
        ])
    else:
        ccflags.extend([
            "-gstabs+",
        ])

    #ccdefs.append("_DEBUG")
    ccdefs.append("DEBUG")
    build_dir = os.path.join(build_dir, "debug")

    # append -debug to our targets
    core_lib += "-debug"
    engine_lib += "-debug"

    tests_app += "-debug"

print("Using variant directory: %s" % build_dir)

# checks for a C++ library and exists if it doesn't exist
def CheckLibOrExit(conf, lib, add=True):
    if not conf.CheckLib(lib, language="C++", autoadd=add):
        print("lib%s not found!" % lib)
        Exit(1)

# checks for the common libraries
def CheckCommonConfiguration(env, check_libs):
    check_libs = True   # bleh :(

    custom_tests = { "CheckBoost" : CheckBoost }

    conf = Configure(env, custom_tests=custom_tests)
    if not conf.env.GetOption("clean"):
        if not conf.CheckBoost(BOOST_VERSION):
            print("Boost version >= %s required!" % BOOST_VERSION)
            Exit(1)

        if conf.CheckCHeader("valgrind/callgrind.h") or conf.CheckCHeader("callgrind.h"):
            conf.env.MergeFlags({ "CPPDEFINES": [ "-DHAS_CALLGRIND_H" ] })

        if check_libs:
            CheckLibOrExit(conf, "curses")
            CheckLibOrExit(conf, "png")
            CheckLibOrExit(conf, "pthread")
            CheckLibOrExit(conf, "ssl")
            CheckLibOrExit(conf, "crypto")

            CheckLibOrExit(conf, "boost_date_time")
            CheckLibOrExit(conf, "boost_filesystem")
            CheckLibOrExit(conf, "boost_system")
            CheckLibOrExit(conf, "boost_thread")

            if "unittest" in build_meta_targets:
                CheckLibOrExit(conf, "cppunit")

            if efence:
                CheckLibOrExit(conf, "efence")

    return conf

# checks for engine libraries
def CheckEngineConfiguration(env, check_libs):
    #check_libs = True   # bleh :(

    conf = CheckCommonConfiguration(env, check_libs)
    if not conf.env.GetOption("clean"):
        if check_libs:
            pass

    return conf

def GenerateCommonEnv(app_name=""):
    env = Environment(CCFLAGS=ccflags, CPPPATH=ccpath, CPPDEFINES=ccdefs,
        LIBPATH=ldpath, LINKFLAGS=ldflags, LIBS=libs)

    if app_name:
        env.MergeFlags({ "CPPDEFINES": [ "-DAPPNAME=\\\"%s\\\"" % app_name ] })

    return env

def GenerateEngineEnv(include_libs, app_name=""):
    env = GenerateCommonEnv(app_name)

    return env

def BuildCore():
    print("Building core library...")

    env = GenerateCommonEnv()
    conf = CheckCommonConfiguration(env, False)
    env = conf.Finish()

    obj = SConscript(os.path.join(src_dir, "SConscript"), exports=[ "env" ],
        variant_dir=build_dir, duplicate=0)
    obj.extend(SConscript(os.path.join(src_dir, "core", "SConscript"), exports=[ "env" ],
        variant_dir=os.path.join(build_dir, "core"), duplicate=0))
    lib = env.StaticLibrary(os.path.join(lib_dir, core_lib), obj)

    Clean(lib, base_build_dir)
    Clean(lib, lib_dir)


def BuildEngine():
    print("Building engine library...")

    env = GenerateEngineEnv(False)
    conf = CheckEngineConfiguration(env, False)
    env = conf.Finish()

    obj = SConscript(os.path.join(src_dir, "engine", "SConscript"), exports=[ "env" ],
        variant_dir=os.path.join(build_dir, "engine"), duplicate=0)
    lib = env.StaticLibrary(os.path.join(lib_dir, engine_lib), obj)

    Clean(lib, base_build_dir)
    Clean(lib, lib_dir)

def BuildTests():
    print("Building tests...")

    env = GenerateCommonEnv(tests_app)
    conf = CheckCommonConfiguration(env, True)
    env = conf.Finish()

    # have to add libs to LINKFLAGS so we can tell the linker
    # to add all of the symbols (or we'll have no tests)
    if sys.platform == "darwin":
        # hey guess what, this doesn't work. thanks a lot apple
        env.MergeFlags({ "LINKFLAGS": [
            "-Wl,-all_load",
            "-l%s" % engine_lib,
            "-l%s" % core_lib,
            "-Wl,-noall_load"
        ] })
    elif "linux" in sys.platform:
        env.MergeFlags({ "LINKFLAGS": [
            "-Wl,-whole-archive",
            "-l%s" % engine_lib,
            "-l%s" % core_lib,
            "-Wl,-no-whole-archive"
        ] })
    else:
        env.MergeFlags({ "LIBS": [ engine_lib, core_lib ] })

    obj = SConscript(os.path.join(src_dir, "test", "SConscript"), exports=[ "env" ],
        variant_dir=os.path.join(build_dir, "test"), duplicate=0)
    app = env.Program(os.path.join(bin_dir, tests_app), obj)

    # have to add libraries as dependencies by hand on linux
    if sys.platform == "darwin" or "linux" in sys.platform:
        Depends(app, os.path.join(lib_dir, "lib%s.a" % engine_lib))
        Depends(app, os.path.join(lib_dir, "lib%s.a" % core_lib))

    Clean(app, base_build_dir)
    Clean(app, bin_dir)

    if "runtests" in build_meta_targets:
        #print("Running tests...")
        #from test.test import main as test_main
        #test_alias = Alias(tests_app, [ app ], test_main(["test/test.py", "-r", "-V"]))
        #AlwaysBuild(test_alias)

        #from test.test import main as test_main
        #print("Tests exited with status: %d" % test_main(["test/test.py", "-r", "-V"]))

        print("TODO: make this work")

# create tags
if "ctags" in build_meta_targets:
    os.system("ctags -R")
    sys.exit(0)

# everything requires core
BuildCore()

if "unittest" in build_meta_targets:
    # build everything that's not an app
    BuildEngine()
    BuildTests()
else:
    BuildEngine()
