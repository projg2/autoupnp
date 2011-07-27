ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libautoupnp.la
bin_SCRIPTS = autoupnp

libautoupnp_la_SOURCES = autoupnp.c notify.c notify.h registry.c registry.h upnp.c upnp.h
libautoupnp_la_LDFLAGS = -module -avoid-version
libautoupnp_la_LIBADD = $(LIBNOTIFY_LIBS)
libautoupnp_la_CPPFLAGS = $(LIBNOTIFY_CFLAGS)
