ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libautoupnp.la
bin_SCRIPTS = autoupnp

libautoupnp_la_SOURCES = src/autoupnp.c src/notify.c src/notify.h \
	src/registry.c src/registry.h src/upnp.c src/upnp.h
libautoupnp_la_LDFLAGS = -module -avoid-version
libautoupnp_la_LIBADD = $(LIBNOTIFY_LIBS)
libautoupnp_la_CPPFLAGS = $(LIBNOTIFY_CFLAGS)
