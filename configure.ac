# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])
AC_INIT(emeralddb,1.0)
AC_USE_SYSTEM_EXTENSIONS
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([include/core.hpp])#自动生成
AC_CONFIG_HEADERS([config.h])#自动生成
# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T
# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRNLEN
AC_CHECK_FUNCS([localtime_r memset pow strchr strtol])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
