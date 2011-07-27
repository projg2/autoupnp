ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libautoupnp.la
dist_bin_SCRIPTS = autoupnp

CC = $(PTHREAD_CC)

libautoupnp_la_SOURCES = src/autoupnp.c src/notify.c src/notify.h \
	src/registry.c src/registry.h src/upnp.c src/upnp.h
libautoupnp_la_LDFLAGS = -module -avoid-version
libautoupnp_la_LIBADD = $(LIBNOTIFY_LIBS) $(PTHREAD_LIBS)
libautoupnp_la_CPPFLAGS = $(LIBNOTIFY_CFLAGS) $(PTHREAD_CFLAGS)
