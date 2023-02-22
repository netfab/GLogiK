# Process this file with autoconf to produce a configure script.

m4_define([GLOGIK_NAME], [GLogiK])
m4_define(GLOGIK_VERSION,regexp(m4_include(VERSION),[version \([-.0-9A-Za-z]+\)],[\1]))

AC_PREREQ([2.59])

# project infos
AC_INIT(GLOGIK_NAME, GLOGIK_VERSION, [https://framagit.org/netfab/GLogiK/issues], GLOGIK_NAME, [https://glogik.tuxfamily.org/])

PKG_PROG_PKG_CONFIG

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_SRCDIR([src/bin/daemon/main.cpp])

dnl Fix:	configure.ac:334: warning: AC_ARG_PROGRAM was called before AC_CANONICAL_TARGET
dnl 		aclocal.m4:2390: AX_PTHREAD is expanded from...
dnl 		configure.ac:334: the top level
dnl
dnl After:	AX_PTHREAD version 29
dnl
dnl			https://git.savannah.gnu.org/gitweb/?p=autoconf-archive.git;
dnl     		a=commitdiff;h=2567e0ce0f3a11b535c6b527386197fb49ff172b
dnl
AC_CANONICAL_TARGET

# Automake initialisation
AM_INIT_AUTOMAKE([foreign subdir-objects])

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

dnl check for overlinking option
AC_ARG_ENABLE( [as-needed],
               AS_HELP_STRING( [--enable-as-needed], [Enable overlinking protection]),
               [enable_as_needed=$enableval], [enable_as_needed=no])

dnl do we want to -Wall flag ?
AC_ARG_ENABLE( [gcc-wall],
               AS_HELP_STRING( [--enable-gcc-wall], [Enable gcc -Wall flag]),
               [enable_gcc_wall=$enableval], [enable_gcc_wall=no])

dnl do we want to -Werror flag ?
AC_ARG_ENABLE( [gcc-werror],
               AS_HELP_STRING( [--enable-gcc-werror], [Enable gcc -Werror flag]),
               [enable_gcc_werror=$enableval], [enable_gcc_werror=no])

AC_ARG_ENABLE( [debug],
				AS_HELP_STRING( [--enable-debug], [Enable debugging, default: no]),
				[enable_debugging=$enableval], [enable_debugging=no])

AC_ARG_ENABLE( [debug-keys],
				AS_HELP_STRING( [--enable-debug-keys], [Enable keys extra debugging, default: no]),
				[enable_keys_debugging=$enableval], [enable_keys_debugging=no])

AC_ARG_ENABLE( [debug-gkdbus],
				AS_HELP_STRING( [--enable-debug-gkdbus], [Enable gkdbus extra debugging, default: no]),
				[enable_gkdbus_debugging=$enableval], [enable_gkdbus_debugging=no])

AC_ARG_ENABLE( [debug-libusb],
				AS_HELP_STRING( [--enable-debug-libusb], [Enable libusb extra debugging, default: no]),
				[enable_libusb_extra_debugging=$enableval], [enable_libusb_extra_debugging=no])

AC_ARG_ENABLE( [debug-pbmfont],
				AS_HELP_STRING( [--enable-debug-pbmfont], [Enable pbmfont extra debugging, default: no]),
				[enable_pbmfont_debugging=$enableval], [enable_pbmfont_debugging=no])

AC_ARG_ENABLE( [debug-lcd-plugins],
				AS_HELP_STRING( [--enable-debug-lcd-plugins], [Enable LCD plugins extra debugging, default: no]),
				[enable_lcd_plugins_debugging=$enableval], [enable_lcd_plugins_debugging=no])

dnl check for DBus support
AC_ARG_ENABLE( [dbus],
               AS_HELP_STRING( [--enable-dbus], [Enable D-Bus support]),
               [enable_dbus=$enableval], [enable_dbus=yes])

dnl check for notifications support
AC_ARG_ENABLE( [notifications],
               AS_HELP_STRING( [--enable-notifications], [Enable notifications support]),
               [enable_notifications=$enableval], [enable_notifications=yes])

dnl check for libnotify support
AC_ARG_ENABLE( [libnotify],
               AS_HELP_STRING( [--enable-libnotify], [Enable libnotify support]),
               [enable_libnotify=$enableval], [enable_libnotify=yes])

dnl check for Qt5 client GUI
AC_ARG_ENABLE( [qt5],
               AS_HELP_STRING( [--enable-qt5], [Enable Qt5 client GUI]),
               [enable_qt5=$enableval], [enable_qt5=no])

dnl check for hidapi support
AC_ARG_ENABLE( [hidapi],
               AS_HELP_STRING( [--enable-hidapi], [Enable hidapi API support]),
               [enable_hidapi=$enableval], [enable_hidapi=yes])

AC_ARG_VAR(DATA_DIR, [Directory where data files will be installed])
AC_ARG_VAR(PBM_DATA_DIR, [Directory where pbm data files will be installed])

dnl -------------------------------------------------------------------------
dnl        User and Group for running the GLogiK daemon
dnl -------------------------------------------------------------------------
dnl user
AC_ARG_WITH(
	[glogikd_user],
	AS_HELP_STRING([--with-glogikd-user=<user>], [User for running the glogik daemon (glogikd)])
)
AS_IF([test -z "$with_glogikd_user"],
		[GLOGIKD_USER=glogikd],
		[GLOGIKD_USER=$with_glogikd_user])
AC_SUBST(GLOGIKD_USER)
AC_DEFINE_UNQUOTED(GLOGIKD_USER,"$GLOGIKD_USER", [User for running the glogik daemon])

dnl group
AC_ARG_WITH(
	[glogikd_group],
	AS_HELP_STRING([--with-glogikd-group=<group>], [Group for running the glogik daemon (usb)])
)
AS_IF([test -z "$with_glogikd_group"],
		[GLOGIKD_GROUP=usb],
		[GLOGIKD_GROUP=$with_glogikd_group])
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

AS_IF([test -z "$with_glogiks_group"],
		[GLOGIKS_GROUP=glogiks],
		[GLOGIKS_GROUP=$with_glogiks_group])
AC_SUBST(GLOGIKS_GROUP)
AC_DEFINE_UNQUOTED(GLOGIKS_GROUP,"$GLOGIKS_GROUP", [Group for running the glogik desktop service])

dnl ---------------------------------------------------------------------------
dnl             Where to install udev rules
dnl ---------------------------------------------------------------------------
AS_IF([test "$prefix" = "/usr" -o "$prefix" = "/usr/local"],
		[udev_rules_dir_default=$($PKG_CONFIG --variable=udevdir udev)"/rules.d"],
		[udev_rules_dir_default=$prefix/lib/udev/rules.d])

AC_ARG_WITH([udevrulesdir],
	AS_HELP_STRING([--with-udevrulesdir=DIR], [Directory where udev expects its rules files]),
	[],
	[with_udevrulesdir=$udev_rules_dir_default]
)
AC_SUBST([udevrulesdir], [$with_udevrulesdir])

dnl -------------------------------------------------------------------------
dnl             run tests and do things accordingly to results
dnl -------------------------------------------------------------------------

GDT_CONDITIONAL_ADD_LDFLAGS([$enable_as_needed], [-Wl,--as-needed])

GDT_CONDITIONAL_ADD_CXXFLAG([$enable_gcc_wall], [-Wall])
GDT_CONDITIONAL_ADD_CXXFLAG([$enable_gcc_werror], [-Werror])

GDT_DEBUG_FLAG(	[$enable_debugging],
				[DEBUGGING_ON],
				[global debugging])

GDT_DEBUG_FLAG(	[$enable_keys_debugging],
				[DEBUG_KEYS],
				[keys extra debugging])

GDT_DEBUG_FLAG(	[$enable_gkdbus_debugging],
				[DEBUG_GKDBUS_SUBOBJECTS],
				[GKDBus extra debugging])

GDT_DEBUG_FLAG(	[$enable_libusb_extra_debugging],
				[DEBUG_LIBUSB_EXTRA],
				[libusb extra debugging])

GDT_DEBUG_FLAG(	[$enable_pbmfont_debugging],
				[DEBUG_PBMFONT],
				[pbmfont extra debugging])

GDT_DEBUG_FLAG(	[$enable_lcd_plugins_debugging],
				[DEBUG_LCD_PLUGINS],
				[LCD plugins extra debugging])


AC_MSG_CHECKING([whether we want D-Bus support])
AS_IF([test "x$enable_dbus" = "xyes"], [
	AC_MSG_RESULT([yes])
	AC_SUBST([INIT_NEED_DBUS], [dbus])
	AC_DEFINE([GKDBUS], [1], [GKDBus enabled])
], [
	AC_MSG_RESULT([no])
	AC_SUBST([INIT_NEED_DBUS], [])
])
AM_CONDITIONAL([WITH_DBUS], [test "x$enable_dbus" = "xyes"])

dnl ---
dnl checking for notifcations support
AC_MSG_CHECKING([whether we want notifications support])
AS_IF([test "x$enable_notifications" = "xyes"], [
	AC_MSG_RESULT([yes])
	AC_DEFINE([HAVE_DESKTOP_NOTIFICATIONS], [1], [desktop notifications enabled])

	AC_MSG_CHECKING([whether we want libnotify support])
	AS_IF([test "x$enable_libnotify" = "xyes"], [
		AC_MSG_RESULT([yes])
		AC_DEFINE([HAVE_LIBNOTIFY], [1], [libnotify enabled])
	], [
		AC_MSG_RESULT([no])
		AC_DEFINE([HAVE_LIBNOTIFY], [0], [libnotify disabled])
		AC_MSG_ERROR([notifications via D-Bus only are currently not implemented])
	])
], [
	AC_MSG_RESULT([no])
	AC_MSG_CHECKING([whether we want libnotify support])
	AS_IF([test "x$enable_libnotify" = "xyes"], [
		AC_MSG_RESULT([yes])
		AC_MSG_ERROR([notifications support is required by libnotify support])
	], [AC_MSG_RESULT([no])])
])
AM_CONDITIONAL([WITH_DESKTOP_NOTIFICATIONS], [test "x$enable_notifications" = "xyes"])

dnl ---
dnl checking for hidapi support
AC_MSG_CHECKING([whether we want hidapi support])
AS_IF([test "x$enable_hidapi" = "xyes"],
	[
		AC_MSG_RESULT([yes])
		AC_DEFINE([GKHIDAPI], [1], [GKHIDAPI enabled])
	], [
		AC_MSG_RESULT([no])
		AC_DEFINE([GKLIBUSB], [1], [GKLIBUSB enabled])
	])
AM_CONDITIONAL([WITH_HIDAPI], [test "x$enable_hidapi" = "xyes"])

dnl ---
dnl checking for Qt5 GUI
AM_CONDITIONAL([WITH_QT5], [test "x$enable_qt5" = "xyes"])

AC_MSG_CHECKING([whether we want Qt5 client GUI])
AS_IF([test "x$enable_qt5" = "xyes"],
	[
		AC_MSG_RESULT([yes])
		AS_IF(
			[test "x$enable_dbus" = "xyes"],
			[],
			[AC_MSG_ERROR([Qt5 client GUI requires D-Bus support enabled])]
		)
		AC_DEFINE([QT5_GUI], [1], [Qt5 client GUI])
	], [
		AC_MSG_RESULT([no])
	])

dnl abort compilation after first error
GDT_ADD_CXXFLAG([-Wfatal-errors])
GDT_ADD_CXXFLAG([-std=c++14])

AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_LDFLAGS)

dnl -------------------------------------------------------------------------
dnl                       defining custom macro variables
dnl -------------------------------------------------------------------------

AC_SUBST(DEBUG_DIR, ["/var/tmp/GKDebug"])
AC_DEFINE_UNQUOTED([DEBUG_DIR], ["${DEBUG_DIR}"], [Directory for debug file])

AC_DEFINE_UNQUOTED([DATA_DIR], ["${prefix}/share"], [Root directory for data files installation])
AC_DEFINE_UNQUOTED([DOC_DIR], ["${docdir}"], [Directory for documentation files installation])
AC_DEFINE_UNQUOTED([PBM_DATA_DIR], ["${prefix}/share/${PACKAGE_NAME}/pbm"], [Directory for pbm file installation])
AC_DEFINE_UNQUOTED([QT_DATA_DIR], ["${prefix}/share/${PACKAGE_NAME}/qt"], [Directory for Qt specific files installation])

dnl #define BOOST_LOG_DYN_LINK
AC_DEFINE_UNQUOTED([BOOST_LOG_DYN_LINK], [], [Link dynamically with boost log libs])


dnl -------------------------------------------------------------------------
dnl                       checking for functions
dnl -------------------------------------------------------------------------

AC_CHECK_FUNCS([getpwnam getgrnam initgroups setresgid setegid setregid setresuid seteuid setreuid])

dnl -------------------------------------------------------------------------
dnl                       checking for libraries
dnl -------------------------------------------------------------------------

GDT_PKG_CHECK_MODULE([LIBUDEV], [libudev])
GDT_PKG_CHECK_MODULE([LIBUSB], [libusb-1.0 >= 1.0.19])

AS_IF([test "x$enable_hidapi" = "xyes"],
	[
		GDT_PKG_CHECK_MODULE([LIBHIDAPI], [hidapi-libusb >= 0.10.0])
	], [])

AS_IF([test "x$enable_dbus" = "xyes"],
	[
		AS_IF([test "x$enable_notifications" = "xyes" && test "x$enable_libnotify" = "xyes"],
			[ GDT_PKG_CHECK_MODULE([LIBNOTIFY], [libnotify >= 0.8.1]) ])
		GDT_PKG_CHECK_MODULE([DBUS], [dbus-1 >= 1.10.18])
		GDT_PKG_CHECK_MODULE([LIBEVDEV], [libevdev])
		dnl SMlib used only by desktop binaries
		PKG_CHECK_MODULES([SM], [sm ice])
		GDT_PKG_MODULE_DEFINE_VERSION([SM], [sm])
		GDT_PKG_MODULE_DEFINE_VERSION([ICE], [ice])
		dnl X libraries used only by user service
		GDT_PKG_CHECK_MODULE([LIBX11], [x11])
		GDT_PKG_CHECK_MODULE([LIBXTST], [xtst])
		AS_IF([test "x$enable_qt5" = "xyes"],
			[ PKG_CHECK_MODULES([QT5], [Qt5Core Qt5Gui Qt5Widgets]) ], [])
	], [])

AS_IF([test "x$enable_qt5" = "xyes"],
	[
		AC_DEFUN(
			[GDT_WARN_QMAKE_PATH],
			[ AC_MSG_WARN([Please export your qmake installation directory to the PATH before running "configure"]) ]
		)

		AC_PATH_PROGS(SED, sed)
		if test -z "${SED}"; then
			AC_MSG_ERROR([sed is required])
		fi

		dnl checking for qmake
		AC_PATH_PROGS(QMAKE5, [qmake], no)
		if test "$QMAKE5" = "no"; then
			GDT_WARN_QMAKE_PATH
			AC_MSG_ERROR([Qt5 qmake not found.])
		else
			dnl checking Qt5 version
			qmake5_test_ver="`${QMAKE5} -v 2>&1 | ${SED} -n -e 's/^Using Qt version \(5\.[[0-9.]]\+\).*$/\1/p'`"
			if test -z "$qmake5_test_ver"; then
				GDT_WARN_QMAKE_PATH
				AC_MSG_ERROR([Wrong Qt5 qmake version.])
	        fi

			AC_DEFINE_UNQUOTED([GK_DEP_QT5_VERSION], ["$qmake5_test_ver"], [detected compile-time Qt5 version])
			qt5_incdirs="`$QMAKE5 -query QT_INSTALL_HEADERS`"

			saved_CPPFLAGS=$CPPFLAGS
			CPPFLAGS="${CPPFLAGS} -I${qt5_incdirs}"
			AC_CHECK_HEADER(QtCore/qconfig.h, [], [AC_MSG_ERROR(qconfig.h header not found.)], [])

			XXX_PROGRAM="
			#include <QtCore/qconfig.h>
			#if QT_REDUCE_RELOCATIONS
			#else
			#error \"Qt not built with -reduce-relocations\"
			#endif
			int main(void) { return 0; }
			"

			AC_MSG_CHECKING([whether Qt5 was built with -reduce-relocations])
			AC_COMPILE_IFELSE(
				[AC_LANG_SOURCE([$XXX_PROGRAM])],
				[
					AC_MSG_RESULT([yes])
					AS_VAR_COPY([temp], [QT5_CFLAGS])
					dnl $ grep -A 4 QT_REDUCE_RELOCATIONS /usr/include/qt5/QtCore/qglobal.h
					AS_VAR_SET([QT5_CFLAGS], ["$temp -fPIC"])
				],
				[AC_MSG_RESULT([no])]
			)

			CPPFLAGS=$saved_CPPFLAGS
		fi
	], [])

AX_BOOST_BASE([1.64.0], [], GDT_PKG_NOT_FOUND([boost libraries]))
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_SERIALIZATION
AX_BOOST_DATE_TIME
AX_BOOST_LOG_SETUP
AX_BOOST_LOG
AX_BOOST_THREAD
AX_PTHREAD

AC_CONFIG_LINKS([NEWS:NEWS.md README:README.md])

AC_CONFIG_FILES([
	Makefile
	data/init/openrc/glogikd
	data/udev/99-GLogiK.rules
	data/tmpfiles.d/GLogiK.conf
	])

AS_IF([test "x$enable_dbus" = "xyes"],
	[AC_CONFIG_FILES([data/dbus/GLogiK.conf])],
	[])

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
         docdir : ${docdir}

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
             libnotify LIBS : ${LIBNOTIFY_LIBS}
           libnotify CFLAGS : ${LIBNOTIFY_CFLAGS}
                hidapi LIBS : ${LIBHIDAPI_LIBS}
              hidapi CFLAGS : ${LIBHIDAPI_CFLAGS}
              libusb CFLAGS : ${LIBUSB_CFLAGS}
                libusb LIBS : ${LIBUSB_LIBS}
               libudev LIBS : ${LIBUDEV_LIBS}
             pthread CFLAGS : ${PTHREAD_CFLAGS}
               pthread LIBS : ${PTHREAD_LIBS}
             Boost CPPFLAGS : ${BOOST_CPPFLAGS}
              Boost LDFLAGS : ${BOOST_LDFLAGS}
   Boost serialization LIBS : ${BOOST_SERIALIZATION_LIB}
       Boost log setup LIBS : ${BOOST_LOG_SETUP_LIB}
             Boost log LIBS : ${BOOST_LOG_LIB}
          Boost thread LIBS : ${BOOST_THREAD_LIB}
          Boost system LIBS : ${BOOST_SYSTEM_LIB}
      Boost filesystem LIBS : ${BOOST_FILESYSTEM_LIB}
 Boost program options LIBS : ${BOOST_PROGRAM_OPTIONS_LIB}
       Boost iostreams LIBS : ${BOOST_IOSTREAMS_LIB}
                   X11 LIBS : ${LIBX11_LIBS}
                 X11 CFLAGS : ${LIBX11_CFLAGS}
                  XTST LIBS : ${LIBXTST_LIBS}
                XTST CFLAGS : ${LIBXTST_CFLAGS}
                   Qt5 LIBS : ${QT5_LIBS}
                 Qt5 CFLAGS : ${QT5_CFLAGS}
--------------------------------------------------------------
        with Qt5 client GUI : ${enable_qt5}
          found Qt5 version : ${qmake5_test_ver}
--------------------------------------------------------------
---  DEBUGGING --- DEBUGGING ---- DEBUGGING --- DEBUGGING  ---
--------------------------------------------------------------
  Global debugging : ${enable_debugging}
              keys : ${enable_keys_debugging}
            GKDBus : ${enable_gkdbus_debugging}
            libusb : ${enable_libusb_extra_debugging}
           PBMFont : ${enable_pbmfont_debugging}
       LCD plugins : ${enable_lcd_plugins_debugging}
         debug dir : ${DEBUG_DIR}
--------------------------------------------------------------

])

