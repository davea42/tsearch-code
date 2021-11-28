###Copyright (C) 2018 Vincent Torri <vincent dot torri at gmail dot com>
###This code is public domain and can be freely used or copied.

dnl defines the version name of  the libdwarf.so
m4_define([v_maj], [0])
m4_define([v_min], [1])
m4_define([v_mic], [0])
m4_define([v_ver], [v_maj.v_min.v_mic])

m4_define([v_rel], [])
m4_define([lt_cur], [m4_eval(v_maj + v_min)])
m4_define([lt_rev], [v_mic])
m4_define([lt_age], [v_min])

### Sets the release name.
###m4_define([v_date], [m4_esyscmd_s([date "+%Y%m%d"])])
###Also SET (VERSION ..) in CMakeLists.txt

AC_PREREQ([2.52])
### 2nd arg to AC_INIT is the version 'number'.
AC_INIT([tsearch], [v_ver], [libdwarf-list -at- linuxmail -dot- org])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

### Additional options to configure

AC_ARG_ENABLE([sanitize],
   [AS_HELP_STRING([--enable-sanitize],
                   [enable sanitize compiler option @<:@default=no@:>@])],
   [
    AS_IF(
        [test "x${enableval}" = "xyes"],
        [enable_sanitize="yes"],
        [enable_sanitize="no"])
   ],
   [enable_sanitize="no"])

AC_ARG_ENABLE([wall],
   [AS_HELP_STRING([--enable-wall],
                   [enable -Wall and other options @<:@default=no@:>@])],
   [
    AS_IF(
        [test "x${enableval}" = "xyes"],
        [enable_wall="yes"],
        [enable_wall="no"])
   ],
   [enable_wall="no"])

### Default options with respect to host

AC_CANONICAL_HOST

have_win32="no"
case "$host_os" in
   mingw*)
      have_win32="yes"
   ;;
esac

AM_CONDITIONAL([HAVE_WIN32], [test "x${have_win32}" = "xyes"])


### Checks for programs

AM_PROG_AR

### We don't use dist-xz *.xz output from make dist,
### so don't mention it.
AM_INIT_AUTOMAKE([1.6 dist-xz])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AS_IF(
    [test "x${have_win32}" = "xyes"],
    [lt_cv_deplibs_check_method=pass_all])
LT_INIT([win32-dll disable-shared static])

AS_IF(
    [test "x${have_win32}" = "xyes" && test "x${enable_static}" = "xyes"],
    [AC_MSG_ERROR([Static library is not supported on Windows])])

version_info="lt_cur:lt_rev:lt_age"
release_info="v_rel"
AC_SUBST([version_info])
AC_SUBST([release_info])

VMAJ=v_maj
AC_SUBST([VMAJ])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CC_C_O

PKG_PROG_PKG_CONFIG

### Checks for libraries

### Checks for header files

### for use in casts to uint to avoid 32bit warnings.
### Also needed by C++ cstdint
AC_TYPE_UINTPTR_T
AC_TYPE_INTPTR_T
AC_TYPE_UINT32_T dnl Needed for hash calculation
### Now we know uintptr_t is either in stdint.h or
### is defined in config.h by configure.

# test Elf headers in the preprocessor path search
# We need Elf.h defines like Ehdr32 and relocations
# to read them with our local elf-section reader code.
# On Solaris libelf.h has relocation defines we need.
# so we check for libelf.h too.
CPPFLAGS_save=${CPPFLAGS}
# dwarfgen needs an actual libelf else it cannot be built.
### we set $dwarf_with_libelf above.

### Checks for compiler characteristics

AC_C_BIGENDIAN
AC_SUBST([DWARF_BIGENDIAN],[${ac_cv_c_bigendian}])
AC_ARG_VAR([DWARF_BIGENDIAN],
    [big endian yes/no needed various places])


# gcc accepts even totally bogus -Wno flags. Other compilers..no
# -Wno-long-long suppresses warnings on 'long long'
# -Wno-pedantic-ms-format (which only exists in mingw)
# suppresses warnings about I64 printf format.
c_common_flags="-Wall -Wextra -Wpointer-arith -Wmissing-declarations -Wcomment -Wformat -Wpedantic -Wuninitialized -Wshadow -Werror -Wno-long-long"
AS_IF(
    [ test "x$enable_wall" = "xyes" ],
    [
       cxx_compiler_flags="$c_common_flags -Wno-unused-private-field"

       c_compiler_flags="$c_common_flags -Wmissing-prototypes -Wdeclaration-after-statement -Wbad-function-cast -Wmissing-parameter-type -Wnested-externs"
    ]
    )
AS_IF(
    [ test "x$have_win32" = "xyes" ],
    [
       cxx_compiler_flags="$cxx_compiler_flags -Wno-pedantic-ms-format"

       c_compiler_flags="$c_compiler_flags -Wno-pedantic-ms-format"
    ]
    )

DWARF_CHECK_CXX_COMPILER_FLAGS([${cxx_compiler_flags}])
DWARF_CHECK_C_COMPILER_FLAGS([${c_compiler_flags}])

# unused attribute
AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
         [[
static unsigned int foo(unsigned int x, __attribute__ ((unused)) int y){
    unsigned int x2 = x + 1;
    return x2;
}
int goo() {
    unsigned int y = 0;
    y = foo(12, y);
}
         ]],
         [[
         ]])
    ],
    [
     have_unused="yes"
     AC_DEFINE(
         [HAVE_UNUSED_ATTRIBUTE], [1],
         [Set to 1 if __attribute__ ((unused)) is available.])
    ],
    [have_unused="no"])

AC_MSG_CHECKING([whether "unused" attribute is available])
AC_MSG_RESULT([${have_unused}])

# sanitize
AS_IF(
    [test "x${enable_sanitize}" = "xyes"],
    [
     CFLAGS_save=${CFLAGS}
     CFLAGS="${CFLAGS} -fsanitize=address -fsanitize=leak -fsanitize=undefined -fno-omit-frame-pointer"
     AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM([[]], [[]])],
         [
          enable_sanitize="yes"
          DWARF_CFLAGS="$DWARF_CFLAGS -fsanitize=address -fsanitize=leak -fsanitize=undefined -fno-omit-frame-pointer"
          LDFLAGS="$LDFLAGS -fsanitize=address -fsanitize=leak -fsanitize=undefined -fno-omit-frame-pointer"
         ],
         [enable_sanitize="no"])
     CFLAGS="${CFLAGS_save} ${DWARF_CFLAGS}"
    ])

AC_MSG_CHECKING([whether sanitize options are used])
AC_MSG_RESULT([${enable_sanitize}])

### Checks for linker characteristics

### Checks for library functions

### Checks for system services

AC_CONFIG_FILES([
Makefile
src/Makefile
])

AC_OUTPUT

echo
echo "$PACKAGE $VERSION"
echo
echo "Configuration Options Summary:"
echo
echo "  BuildOS..............: ${build_os}"
echo "  HostOS...............: ${host_os}"
echo
echo "  sanitize support.....: ${enable_sanitize}"
echo "  BuildOS-BigEndian....: ${ac_cv_c_bigendian}"
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS ${c_compiler_flags}"
echo "  LDFLAGS..............: $LDFLAGS"
echo "  LIBS.................: $LIBS"
echo "  ZLIB_LIBS............: $ZLIB_LIBS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...............: $prefix"
echo
