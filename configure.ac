# Process this file with autoconf to produce a configure script.

m4_define([GLOGIK_NAME], [GLogiK])
m4_define(GLOGIK_VERSION,regexp(m4_include(VERSION),[version \([-.0-9A-Za-z]+\)],[\1]))

# project infos
AC_INIT(GLOGIK_NAME, GLOGIK_VERSION, [https://framagit.org/netfab/GLogiK/issues], GLOGIK_NAME)

PKG_PROG_PKG_CONFIG

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_SRCDIR([src/bin/daemon/main.cpp])

# Automake initialisation
AM_INIT_AUTOMAKE([subdir-objects])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CXXCPP
AC_LANG(C++)
AX_CXX_COMPILE_STDCXX_14
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

AC_ARG_ENABLE( [debug-gkdbus],
				AC_HELP_STRING( [--enable-debug-gkdbus], [Enable gkdbus debugging, default: no]),
				[enable_gkdbus_debugging=$enableval], [enable_gkdbus_debugging=no])

AC_ARG_VAR(DEBUG_DIR, [Directory where debug file will be written])

dnl -------------------------------------------------------------------------
dnl        User and Group for running the GLogiK daemon
dnl -------------------------------------------------------------------------
dnl user
AC_ARG_WITH(
	[glogikd_user],
	AS_HELP_STRING([--with-glogikd-user=<user>], [User for running the glogik daemon (glogikd)])
)
if test -z "$with_glogikd_user" ; then
	GLOGIKD_USER=glogikd
else
	GLOGIKD_USER=$with_glogikd_user
fi
AC_SUBST(GLOGIKD_USER)
AC_DEFINE_UNQUOTED(GLOGIKD_USER,"$GLOGIKD_USER", [User for running the glogik daemon])

dnl group
AC_ARG_WITH(
	[glogikd_group],
	AS_HELP_STRING([--with-glogikd-group=<group>], [Group for running the glogik daemon (plugdev)])
)
if test -z "$with_glogikd_group" ; then
	GLOGIKD_GROUP=plugdev
else
	GLOGIKD_GROUP=$with_glogikd_group
fi
AC_SUBST(GLOGIKD_GROUP)
AC_DEFINE_UNQUOTED(GLOGIKD_GROUP,"$GLOGIKD_GROUP", [Group for running the glogik daemon])

AC_ARG_VAR(DEVICES_MODE, [Permissions of detected devices and for virtual keyboard. Defaults to 0660])
AS_IF([test -z "${DEVICES_MODE}"], AC_SUBST([DEVICES_MODE], [0660]))

dnl -------------------------------------------------------------------------
dnl        Group for running the GLogiK desktop service
dnl -------------------------------------------------------------------------

AC_ARG_WITH(
	[glogiks_group],
	AS_HELP_STRING([--with-glogiks-group=<group>], [Group for running the glogik desktop service (glogiks)])
)
if test -z "$with_glogiks_group" ; then
	GLOGIKS_GROUP=glogiks
else
	GLOGIKS_GROUP=$with_glogiks_group
fi
AC_SUBST(GLOGIKS_GROUP)
AC_DEFINE_UNQUOTED(GLOGIKS_GROUP,"$GLOGIKS_GROUP", [User for running the glogik desktop service])

dnl ---------------------------------------------------------------------------
dnl             Where to install udev rules
dnl ---------------------------------------------------------------------------
if test "$prefix" = "/usr" -o "$prefix" = "/usr/local" ; then
	dnl AC_SUBST([udevrulesdir], [$($PKG_CONFIG --variable=udevdir udev)/rules.d])
	udev_rules_dir_default=$($PKG_CONFIG --variable=udevdir udev)"/rules.d"
else
	dnl AC_SUBST([udevrulesdir], [$prefix/lib/udev/rules.d])
	udev_rules_dir_default=$prefix/lib/udev/rules.d
fi

AC_ARG_WITH(
	[udevrulesdir],
	AS_HELP_STRING([--with-udevrulesdir=DIR], [Directory where udev expects its rules files]),
	[],
	[with_udevrulesdir=$udev_rules_dir_default]
)
AC_SUBST([udevrulesdir], [$with_udevrulesdir])

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

	AC_MSG_CHECKING([whether we want gkdbus debugging])
	if test "$enable_gkdbus_debugging" = yes; then
		AC_MSG_RESULT([yes])
		AC_DEFINE_UNQUOTED([DEBUG_GKDBUS_SUBOBJECTS], [1], [Also debug GKDBus sub-objects])
	else
		AC_MSG_RESULT([no])
		AC_DEFINE_UNQUOTED([DEBUG_GKDBUS_SUBOBJECTS], [0], [Also debug GKDBus sub-objects])
	fi
else
	AC_MSG_RESULT([no])
	AC_DEFINE_UNQUOTED([DEBUGGING_ON], [0], [Debugging])
	AC_DEFINE_UNQUOTED([DEBUG_GKDBUS_SUBOBJECTS], [0], [Also debug GKDBus sub-objects])
fi

dnl abort compilation after first error
AS_VAR_APPEND(AM_CXXFLAGS,[" -Wfatal-errors"])
AS_VAR_APPEND(AM_CXXFLAGS,[" -std=c++14"])

AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_LDFLAGS)

dnl -------------------------------------------------------------------------
dnl                       defining custom macro variables
dnl -------------------------------------------------------------------------

AS_IF([[test "x$DEBUG_DIR" = "x"]], [DEBUG_DIR="/tmp"])
AC_DEFINE_UNQUOTED([DEBUG_DIR], ["${DEBUG_DIR}"], [Directory for debug file])

dnl -------------------------------------------------------------------------
dnl                       checking for functions
dnl -------------------------------------------------------------------------

AC_CHECK_FUNCS([getpwnam getgrnam initgroups setresgid setegid setregid setresuid seteuid setreuid])

dnl -------------------------------------------------------------------------
dnl                       checking for libraries
dnl -------------------------------------------------------------------------

PKG_CHECK_MODULES([LIBUSB], [libusb-1.0 >= 1.0.0])
PKG_CHECK_MODULES([LIBUDEV], [libudev])
PKG_CHECK_MODULES([LIBEVDEV], [libevdev])
PKG_CHECK_MODULES([DBUS], [dbus-1 >= 1.10])
PKG_CHECK_MODULES([SM], [sm ice])

AX_BOOST_BASE([1.56.0], [], AC_MSG_ERROR([Could not find a useful version of boost libraries]))
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_IOSTREAMS
AX_BOOST_SERIALIZATION
AX_PTHREAD

AC_CONFIG_FILES([
        Makefile
        data/dbus/GLogiK.conf
        data/udev/99-GLogiK.rules
	])

# Generate config.status and launch it.
AC_OUTPUT

dnl -------------------------------------------------------------------------
dnl                             show summary
dnl -------------------------------------------------------------------------
AC_MSG_NOTICE([
--------------------------------------------------------------
 $PACKAGE_NAME $VERSION configured with :
--------------------------------------------------------------
         prefix : ${prefix}
    exec_prefix : ${exec_prefix}
         libdir : ${libdir}
         bindir : ${bindir}
        sbindir : ${sbindir}
     sysconfdir : ${sysconfdir}
    datarootdir : ${datarootdir}
        datadir : ${datadir}

      pkglibdir : ${libdir}/$PACKAGE
         srcdir : ${srcdir}

 udev rules dir : ${with_udevrulesdir}

       compiler : ${CXX}
       CXXFLAGS : ${CXXFLAGS}
        LDFLAGS : ${LDFLAGS}
    am CXXFLAGS : ${AM_CXXFLAGS}
     am LDFLAGS : ${AM_LDFLAGS}

           $PACKAGE_NAME daemon user : ${GLOGIKD_USER}
          $PACKAGE_NAME daemon group : ${GLOGIKD_GROUP}
 $PACKAGE_NAME desktop service group : ${GLOGIKS_GROUP}
                 devices mode : ${DEVICES_MODE}
--------------------------------------------------------------
                 D-Bus LIBS : ${DBUS_LIBS}
               D-Bus CFLAGS : ${DBUS_CFLAGS}
                    SM LIBS : ${SM_LIBS}
                  SM CFLAGS : ${SM_CFLAGS}
            libevdev CFLAGS : ${LIBEVDEV_CFLAGS}
              libevdev LIBS : ${LIBEVDEV_LIBS}
              libusb CFLAGS : ${LIBUSB_CFLAGS}
                libusb LIBS : ${LIBUSB_LIBS}
               libudev LIBS : ${LIBUDEV_LIBS}
             phtread CFLAGS : ${PTHREAD_CFLAGS}
               phtread LIBS : ${PTHREAD_LIBS}
             Boost CPPFLAGS : ${BOOST_CPPFLAGS}
              Boost LDFLAGS : ${BOOST_LDFLAGS}
   Boost serialization LIBS : ${BOOST_SERIALIZATION_LIB}
          Boost system LIBS : ${BOOST_SYSTEM_LIB}
      Boost filesystem LIBS : ${BOOST_FILESYSTEM_LIB}
 Boost program options LIBS : ${BOOST_PROGRAM_OPTIONS_LIB}
       Boost iostreams LIBS : ${BOOST_IOSTREAMS_LIB}
--------------------------------------------------------------
 Global debugging : ${enable_debugging}
 GKDBus debugging : ${enable_gkdbus_debugging}
        debug dir : ${DEBUG_DIR}
--------------------------------------------------------------

])

