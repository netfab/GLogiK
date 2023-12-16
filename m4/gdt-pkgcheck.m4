dnl
dnl	This file is part of GLogiK project.
dnl	GLogiK, daemon to handle special features on gaming keyboards
dnl	Copyright (C) 2016-2024  Fabrice Delliaux <netbox253@gmail.com>
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

dnl GDT_PKG_NOT_FOUND(PACKAGE)
dnl --------------------------------------------------------------
dnl Print a PACKAGE_NOT_FOUND error message and exit configure
AC_DEFUN([GDT_PKG_NOT_FOUND],
[
	AC_MSG_ERROR([cannot find a useful version of $1])
])

dnl GDT_PKG_MODULE_DEFINE_VERSION(VARIABLE-PREFIX, MODULE)
dnl --------------------------------------------------------------
dnl Run $PKG_CONFIG to get the MODULE version, and use it
dnl to define a C preprocessor macro using VARIABLE-PREFIX
dnl
dnl example:
dnl --------
dnl 	GDT_PKG_MODULE_DEFINE_VERSION([FOO], [libfoo >= 1.2.5])
dnl 		will define following preprocessor symbol:
dnl 	GK_DEP_FOO_VERSION_STRING="2.0.0"
dnl 		if libfoo-2.0.0 is found at compile-time
AC_DEFUN([GDT_PKG_MODULE_DEFINE_VERSION],
[
	AC_REQUIRE([PKG_PROG_PKG_CONFIG])
	AS_IF([test -z "$2"], [AC_MSG_ERROR([missing module parameter])])
	if test -n "$PKG_CONFIG"; then
		pkg_failed=no
		GK_DEP_$1_VERSION_STRING=`$PKG_CONFIG --modversion "$2"`
		test "x$?" != "x0" && pkg_failed=yes
		if test $pkg_failed = no; then
			AC_DEFINE_UNQUOTED(
				[GK_DEP_$1_VERSION_STRING],
				["$GK_DEP_$1_VERSION_STRING"],
				[detected compile-time $1 version]
			)
		else
			GDT_PKG_NOT_FOUND([$1])
		fi
	else
		AC_MSG_ERROR([PKG_CONFIG empty, cannot find $1 version])
	fi
])

dnl GDT_PKG_CHECK_MODULE(VARIABLE-PREFIX, MODULE)
dnl --------------------------------------------------------------
dnl
dnl Run PKG_CHECK_MODULES then GDT_PKG_MODULE_DEFINE_VERSION with
dnl VARIABLE-PREFIX and MODULE parameters
AC_DEFUN([GDT_PKG_CHECK_MODULE],
[
	PKG_CHECK_MODULES([$1], [$2])
	GDT_PKG_MODULE_DEFINE_VERSION([$1], [$2])
])
