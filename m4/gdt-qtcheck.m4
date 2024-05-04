dnl
dnl	This file is part of GLogiK project.
dnl	GLogiK, daemon to handle special features on gaming keyboards
dnl	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
dnl
dnl	This program is free software: you can redistribute it and/or modify
dnl	it under the terms of the GNU General Public License as published by
dnl	the Free Software Foundation, either version 3 of the License, or
dnl	(at your option) any later version.
dnl
dnl	This program is distributed in the hope that it will be useful,
dnl	but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl	GNU General Public License for more details.
dnl
dnl	You should have received a copy of the GNU General Public License
dnl	along with this program.  If not, see <http://www.gnu.org/licenses/>.
dnl

AC_DEFUN([GDT_WARN_QMAKE_PATH],
[
	AC_MSG_WARN([Please export your qmake installation directory to the PATH before running "configure"])
])

dnl GDT_QT_CHECK(prefix, Qt-major-version, list-of-modules)
dnl --------------------------------------------------------------
dnl Check for Qt packages
AC_DEFUN([GDT_QT_CHECK],
[
	PKG_CHECK_MODULES([$1], [$3])

	AC_PATH_PROGS(SED, sed)
	if test -z "${SED}"; then
		AC_MSG_ERROR([sed is required])
	fi

	dnl checking for qmake
	AC_PATH_PROGS(QMAKE, [qmake], no)
	if test "$QMAKE" = "no"; then
		GDT_WARN_QMAKE_PATH
		AC_MSG_ERROR([Qt qmake not found.])
	else
		dnl checking Qt version
		qmake_test_ver="`${QMAKE} -v 2>&1 | ${SED} -n -e 's/^Using Qt version \([$2]\.[[0-9.]]\+\).*$/\1/p'`"
		if test -z "$qmake_test_ver"; then
			GDT_WARN_QMAKE_PATH
			AC_MSG_ERROR([Wrong Qt qmake version.])
	    fi
		AC_MSG_NOTICE([found Qt version: $qmake_test_ver])

		AC_DEFINE_UNQUOTED([GK_DEP_QT_VERSION_STRING], ["$qmake_test_ver"], [detected compile-time Qt version])
		qt_incdirs="`$QMAKE -query QT_INSTALL_HEADERS`"

		saved_CPPFLAGS=$CPPFLAGS
		CPPFLAGS="${CPPFLAGS} -I${qt_incdirs}"
		AC_CHECK_HEADER(QtCore/qconfig.h, [], [AC_MSG_ERROR(qconfig.h header not found.)], [])

		XXX_PROGRAM="
		#include <QtCore/qconfig.h>
		#ifdef QT_REDUCE_RELOCATIONS
		#else
		#error \"Qt not built with -reduce-relocations\"
		#endif
		int main(void) { return 0; }
		"

		AC_MSG_CHECKING([whether Qt was built with -reduce-relocations])
		AC_COMPILE_IFELSE(
			[AC_LANG_SOURCE([$XXX_PROGRAM])],
			[
				AC_MSG_RESULT([yes])
				AS_VAR_COPY([temp], [QT_CFLAGS])
				dnl $ grep -A 4 QT_REDUCE_RELOCATIONS /usr/include/qt5/QtCore/qglobal.h
				AS_VAR_SET([QT_CFLAGS], ["$temp -fPIC"])
			],
			[AC_MSG_RESULT([no])]
		)

		CPPFLAGS=$saved_CPPFLAGS
	fi
])
