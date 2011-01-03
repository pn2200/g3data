#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.66])
AC_INIT([g3data], [1.5.3])

# Put compiled object files into the same sub-directory as their sources
# and quiet make
AM_INIT_AUTOMAKE([1.11 subdir-objects])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([g3data/main.c])
AC_CONFIG_HEADERS([config.h])

WARNING_FLAGS="-W -Wall -Wshadow -Wcast-align -Wredundant-decls -Wbad-function-cast -Wcast-qual -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes"
AC_SUBST(WARNING_FLAGS)

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

# Checks for libraries.
PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([GTK], [gtk+-2.0])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strstr])

AC_CONFIG_FILES([Makefile
                 doc/Makefile])
AC_OUTPUT
