AC_PREREQ([2.60])
AC_INIT([autoupnp], [0.4.6])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.6 foreign dist-bzip2 no-dependencies -Wall])

AC_PROG_CC
AC_USE_SYSTEM_EXTENSIONS
LT_INIT([disable-static])
ACX_PTHREAD

AC_SEARCH_LIBS([dlopen], [dl],, [
	AC_MSG_ERROR([dlopen() not found])
])

AC_CHECK_LIB([miniupnpc], [UPNP_AddPortMapping],, [
	AC_MSG_ERROR([libminiupnpc not found, http://miniupnp.free.fr/])
])

AC_ARG_WITH([libnotify],
	[AS_HELP_STRING([--without-libnotify],
		[Disable use of libnotify])])
AS_IF([test x"$with_libnotify" != x"no"],
	[PKG_CHECK_MODULES([LIBNOTIFY], [libtinynotify], [
		AC_DEFINE([HAVE_LIBNOTIFY], [1], [define if you have libtinynotify])
	], [
		AC_MSG_WARN([libtinynotify not found, disabling.])
		with_libnotify=no
])])

AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
