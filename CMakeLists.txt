cmake_minimum_required(VERSION 3.0.0)

include (CheckSymbolExists)

project (Zile VERSION 2.4.14 )

set(CMAKE_VERBOSE_MAKEFILE ON)

option (enable-gcc-warnings "Enable gcc warnings")
if (enable-gcc-warnings)
  add_compile_options (-Wall)
endif ()

option (enable-mouse "Enable gcc warnings" ON)
if (enable-mouse)
  add_definitions(-DMOUSE_ON=1)
endif ()

find_program (HELP2MAN help2man)

check_symbol_exists ("re_search" regex.h SYS_RE_SEARCH)

# Ncurses
find_package (Curses REQUIRED)
include_directories (${CURSES_INCLUDE_DIR})

find_package(PkgConfig REQUIRED)
# Libgc
pkg_search_module (LIBGC REQUIRED bdw-gc>=7.2)

find_program (PERL perl)
if (NOT PERL)
  message (FATAL_ERROR "Cannot find perl executable in the path.")
endif ()

find_program (EMACS emacs)
if (EMACS)
  execute_process(
    COMMAND ${EMACS} --version 2>&1
    OUTPUT_VARIABLE EMACS_VERSION)
endif ()

string (REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" EMACS_VERSION ${EMACS_VERSION})

message ("-- Emacs version: ${EMACS_VERSION}")

include_directories(lib)

# gnulib

set (gnulib_modules
  announce-gen
  array-list
  chown
  copy-file
  dirname
  do-release-commit-and-tag
  euidaccess
  fcntl
  gendocs
  getcwd
  getopt-gnu
  gettimeofday
  gitlog-to-changelog
  gnupload
  hash
  linked-list
  maintainer-makefile
  manywarnings
  memmem
  minmax
  pipe-filter-ii
  pmccabe2html
  progname
  readme-release
  regex
  size_max
  snippet/unused-parameter
  stdarg
  stdbool
  valgrind-tests
  warnings
  xalloc-die
  xgetcwd
  xlist
  xvasprintf-posix)

include (ExternalProject)

ExternalProject_Add(gnulib
  DOWNLOAD_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gnulib/gnulib-tool --create-testdir --dir=${CMAKE_CURRENT_BINARY_DIR}/gnulib ${gnulib_modules}
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/gnulib # where ./configure is executed
  CONFIGURE_COMMAND ./configure
  PREFIX gnulib
  BUILD_COMMAND ${MAKE}
  )

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/gnulib/gllib)

include (GNUInstallDirs)

configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(src)
