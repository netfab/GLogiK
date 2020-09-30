dnl
dnl	This file is part of GLogiK project.
dnl	GLogiK, daemon to handle special features on gaming keyboards
dnl	Copyright (C) 2016-2020  Fabrice Delliaux <netbox253@gmail.com>
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


AC_DEFUN([GDT_ADD_CXXFLAG],
[
	dnl AS_VAR_APPEND(AM_CXXFLAGS,[ $1])
	AS_VAR_COPY([temp], [AM_CXXFLAGS])
	AS_VAR_SET([AM_CXXFLAGS], ["$temp $1"])
])

AC_DEFUN([GDT_CONDITIONAL_ADD_CXXFLAG],
[
	AC_MSG_CHECKING([whether we want CXX flag $2])
	AS_IF([test "x$1" = "xyes"],
		[
			AC_MSG_RESULT([yes])
			GDT_ADD_CXXFLAG([$2])
		], [
			AC_MSG_RESULT([no])
		]
	)
])

AC_DEFUN([GDT_CONDITIONAL_ADD_LDFLAGS],
[
	AC_MSG_CHECKING([whether we want linker flag $2])
	AS_IF([test "x$1" = "xyes"],
		[
			AC_MSG_RESULT([yes])
			AX_APPEND_LINK_FLAGS([$2], [AM_LDFLAGS])
		], [
			AC_MSG_RESULT([no])
		]
	)
])

AC_DEFUN([GDT_DEBUG_FLAG],
[
	AC_MSG_CHECKING([whether we want $3])
	AS_IF([test "x$enable_debugging" = "xyes"],
		[
			AS_IF([test "x$1" = "xyes"],
				[
					AC_MSG_RESULT([yes])
					AC_DEFINE([$2], [1], [$3])
				], [
					AC_MSG_RESULT([no])
					AC_DEFINE([$2], [0], [$3])
				]
			)
		], [
			AS_IF([test "x$3" = "xglobal debugging"],
				[
					AC_MSG_RESULT([no])
				], [
						AS_IF([test "x$1" = "xyes"],
							[
								AC_MSG_ERROR([$3 requires global debugging])
							], [
								AC_MSG_RESULT([no])
								AC_DEFINE([$2], [0], [$3])
							]
						)
					]
			)
		]
	)
])

