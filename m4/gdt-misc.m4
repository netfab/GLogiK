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

dnl GDT_OPTDEP_CHECK(FEATURE1, option1, FEATURE2, option2)
dnl --------------------------------------------------------------
dnl To get FEATURE1 support (with option1 enabled),
dnl    FEATURE2 support must be enabled with option2
dnl else exit with error
AC_DEFUN([GDT_OPTDEP_CHECK],
[
	AC_MSG_CHECKING([whether we want $1])
	AS_IF([test "x$2" = "xyes"],
		[
			AC_MSG_RESULT([yes])
			AS_IF(
				[test "x$4" = "xyes"],
				[],
				[AC_MSG_ERROR([$1 requires $3 support enabled])]
			)
		], [
			AC_MSG_RESULT([no])
		])
])

dnl GDT_OPTEXCL_CHECK(FEATURE1, option1, FEATURE2, option2)
dnl --------------------------------------------------------------
dnl option1 and option2 are mutually exclusive
AC_DEFUN([GDT_OPTEXCL_CHECK],
[
	AS_IF([test "x$2" = "xyes" -a "x$4" = "xyes"], AC_MSG_ERROR([$1 and $3 are mutually exclusive]), [])
])
