#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([transfer], [1.0], [transfer@bug.com])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign subdir-objects])
LT_INIT
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-ltest':
#AC_CHECK_LIB([test], [main])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_SUBDIRS([libs/jansson-2.11])
# AX_SUBDIRS_CONFIGURE([libs/libevent-2.1.8], [[--disable-samples], [--disable-openssl]])
# AX_SUBDIRS_CONFIGURE([libs/zlib-1.2.11])
# AX_SUBDIRS_CONFIGURE([libs/ncurses-6.1],[[--with-libtool=`pwd`/libtool], [--with-shared], [--without-curses-h], [CPPFLAGS=-P]])
AX_SUBDIRS_CONFIGURE([libs/glib-2.58.1], [[--with-threads=posix], [--with-pcre=internal]])

AC_CONFIG_FILES([Makefile
                 libs/Makefile
                 libs/ccan/Makefile
                 libs/zebra/Makefile
                 src/Makefile])
AC_OUTPUT
