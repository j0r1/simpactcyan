from distutils import sysconfig
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize
import platform
import sys

isDebug = False
if "-g" in sys.argv[1:] or "--debug" in sys.argv[1:]:
    isDebug = True

extraFlags = [ ]
extraIncludes = [ ]
extraDefines = [ ]
libDirs = [ ]
if platform.system() != "Windows":
    cflags = sysconfig.get_config_var('CFLAGS')
    opt = sysconfig.get_config_var('OPT')
    sysconfig._config_vars['CFLAGS'] = cflags.replace(' -g ', ' ').replace('-DNDEBUG', ' ').replace(' -O2', ' ').replace('-O ', ' ').replace('-O3', ' ')
    sysconfig._config_vars['OPT'] = opt.replace(' -g ', ' ').replace('-DNDEBUG', ' ').replace(' -O2', ' ').replace('-O ', ' ').replace('-O3', ' ')
    if isDebug:
        extraFlags = [ "-g", "-std=c++11" ]
    else:
        extraFlags = [ "-DNDEBUG", "-O3", "-std=c++11" ]
    libraries = [ "gsl", "gslcblas" ]
else:
    libDirs = [ "c:\\local\\lib\\" ]
    extraIncludes = [ "c:\\local\\include" ] 
    extraDefines = [ ("WIN32", None) ] 
    libraries = [ "gsl", "cblas" ]

simpactFiles = [ "simpactbindings.cpp",
          "../../lib/core/personaleventlist.cpp",
          "../../lib/core/personaleventlisttesting.cpp",
          "../../lib/core/personbase.cpp",
          "../../lib/core/populationalgorithmadvanced.cpp",
          "../../lib/core/populationalgorithmsimple.cpp",
          "../../lib/core/populationalgorithmtesting.cpp",
          "../../lib/core/populationevent.cpp",
          "../../lib/core/populationstateadvanced.cpp",
          "../../lib/core/populationstatesimple.cpp",
          "../../lib/core/populationstatesimpleadvancedcommon.cpp",
          "../../lib/core/populationstatetesting.cpp",
          "../../lib/core/populationutil.cpp",
          "../../lib/mnrm/algorithm.cpp",
          "../../lib/mnrm/booltype.cpp",
          "../../lib/mnrm/debugtimer.cpp",
          "../../lib/mnrm/eventbase.cpp",
          "../../lib/mnrm/gslrandomnumbergenerator.cpp",
          "../../lib/mnrm/simplealgorithm.cpp",
          "../../lib/util/util.cpp",
        ]

if not isDebug:
    modName = "simpact"
else:
    modName = "simpactdebug"

modFile = modName + ".pyx"

data = open("simpacttemplate.pyx", "rt").read()
open(modFile, "wt").write(data)

extensions = [
    Extension(modName, 
        [ modFile ] + simpactFiles,
        include_dirs = [ "../../lib/core", "../../lib/mnrm", "../../lib/util/" ] + extraIncludes,
        libraries = libraries,
        library_dirs = libDirs,
        language = "c++",
        define_macros = [ ( "DISABLEOPENMP", None), ("EVENTBASE_ALWAYS_CHECK_NANTIME", None), ("NODEBUGTIMER", None) ] + extraDefines,
        extra_compile_args = extraFlags
    ),
]

versionStr = [ l for l in open("../../../CMakeLists.txt").readlines() if "set(VERSION" in l ][0].replace(")","").split()[1].strip()
setup(name = modName, version = versionStr, ext_modules = cythonize(extensions))

