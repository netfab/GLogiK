# Process this file with autoconf to produce a configure script.

m4_define([GLOGIK_NAME], [GLogiK])
m4_define(GLOGIK_VERSION,regexp(m4_include(VERSION),[version \([-.0-9A-Za-z]+\)],[\1]))

# project infos
AC_INIT(GLOGIK_NAME, GLOGIK_VERSION, [netbox253@gmail.com])

AC_CONFIG_HEADERS([config.h])

#AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_SRCDIR([src/bin/daemon/main.cpp])

# Automake initialisation
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CXXCPP
AC_LANG(C++)
AX_CXX_COMPILE_STDCXX_11
AC_PROG_INSTALL
AC_PROG_LN_S

# Checks for headers
#AC_CHECK_HEADERS(climits)

# libtool requirement and setup
LT_PREREQ([2.4.6])
LT_INIT([disable-static])

dnl C++ flags
#AM_CXXFLAGS="-Wall"

dnl preprocessor arguments
#AM_CPPFLAGS="-I\$(top_srcdir)/src/libs -I\$(top_builddir)/src/include"
AM_CPPFLAGS="-I\$(top_srcdir)/src"
AC_SUBST(AM_CPPFLAGS)



dnl -------------------------------------------------------------------------
dnl                     declare custom configure options
dnl -------------------------------------------------------------------------

dnl enable -Wl,--as-needed by default to prevent overlinking
AC_ARG_ENABLE( [as-needed],
               AC_HELP_STRING( [--enable-as-needed], [Enable overlinking protection]),
               [enable_as_needed=$enableval], [enable_as_needed=no])

dnl do we want to -Wall flag ?
AC_ARG_ENABLE( [gcc-wall],
               AC_HELP_STRING( [--enable-gcc-wall], [Enable gcc -Wall flag]),
               [enable_gcc_wall=$enableval], [enable_gcc_wall=no])

dnl do we want to -Werror flag ?
AC_ARG_ENABLE( [gcc-werror],
               AC_HELP_STRING( [--enable-gcc-werror], [Enable gcc -Werror flag]),
               [enable_gcc_werror=$enableval], [enable_gcc_werror=no])

AC_ARG_ENABLE( [debug],
				AC_HELP_STRING( [--enable-debug], [Enable debugging, default: no]),
				[enable_debugging=$enableval], [enable_debugging=no])

AC_ARG_VAR(DEBUG_DIR, [Directory where debug file will be written])


dnl -------------------------------------------------------------------------
dnl             run tests and do things accordingly to results
dnl -------------------------------------------------------------------------

AC_MSG_CHECKING([whether we want overlinking protection])
if test "$enable_as_needed" = yes; then
    AC_MSG_RESULT([yes])
    AX_APPEND_LINK_FLAGS([-Wl,--as-needed], [AM_LDFLAGS])
else
    AC_MSG_RESULT([no])
fi

AC_MSG_CHECKING([whether we want gcc -Wall])
if test "$enable_gcc_wall" = yes; then
    AC_MSG_RESULT([yes])
    AS_VAR_APPEND(AM_CXXFLAGS,["-Wall"])
else
    AC_MSG_RESULT([no])
fi

AC_MSG_CHECKING([whether we want gcc -Werror])
if test "$enable_gcc_werror" = yes; then
    AC_MSG_RESULT([yes])
    AS_VAR_APPEND(AM_CXXFLAGS,[" -Werror"])
else
    AC_MSG_RESULT([no])
fi

AC_MSG_CHECKING([whether we want debugging])
if test "$enable_debugging" = yes; then
    AC_MSG_RESULT([yes])
	AC_DEFINE_UNQUOTED([DEBUGGING_ON], [1], [Debugging])
else
    AC_MSG_RESULT([no])
fi


AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_LDFLAGS)

dnl -------------------------------------------------------------------------
dnl                       defining macro variables
dnl -------------------------------------------------------------------------

AS_IF([[test "x$DEBUG_DIR" = "x"]], [DEBUG_DIR="/tmp"])
AC_DEFINE_UNQUOTED([DEBUG_DIR], ["${DEBUG_DIR}"], [Directory for debug file])

dnl -------------------------------------------------------------------------
dnl -------------------------------------------------------------------------
dnl -------------------------------------------------------------------------

#PKG_CHECK_MODULES([SDL], [sdl >= 1.2.13])
#PKG_CHECK_MODULES([SDLMIXER], [SDL_mixer >= 1.2.11])
PKG_CHECK_MODULES([LIBUSB], [libusb-1.0 >= 1.0.0])
PKG_CHECK_MODULES([LIBUDEV], [libudev])
PKG_CHECK_MODULES([LIBEVDEV], [libevdev])
PKG_CHECK_MODULES([DBUS], [dbus-1 >= 1.10])

AX_BOOST_BASE([1.56.0])
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_IOSTREAMS
AX_PTHREAD

AC_CONFIG_FILES([
        Makefile
        src/Makefile
        src/include/Makefile
        src/lib/Makefile
        src/lib/dbus/Makefile
        src/bin/Makefile
        src/bin/daemon/Makefile
	])

# Generate config.status and launch it.
AC_OUTPUT

dnl -------------------------------------------------------------------------
dnl                             show summary
dnl -------------------------------------------------------------------------
AC_MSG_NOTICE([

 $PACKAGE_NAME $VERSION configured with :
-------------------------------
        prefix:                 ${prefix}
        exec_prefix:            ${exec_prefix}
        libdir:                 ${libdir}
        bindir:                 ${bindir}
        sysconfdir:             ${sysconfdir}
        datadir:                ${datadir}

        pkglibdir:              ${libdir}/$PACKAGE
        srcdir:                 ${srcdir}

        compiler:               ${CXX}
        CXXFLAGS:               ${CXXFLAGS}
        LDFLAGS:                ${LDFLAGS}

])

