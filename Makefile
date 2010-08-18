LIB = autoupnp.so
OBJS = autoupnp.o notify.o privdrop.o registry.o upnp.o

WANT_LIBNOTIFY = true

LCFLAGS = -fPIC \
	$$($(WANT_LIBNOTIFY) && pkg-config --cflags libnotify && printf '%s' '-DHAVE_LIBNOTIFY')
# pthread-stubs may be required or may be not... try to use it anyway.
LLIBS = -ldl -lminiupnpc \
	$$($(WANT_LIBNOTIFY) && pkg-config --libs libnotify) \
	$$(pkg-config --libs pthread-stubs 2>/dev/null)

all:
	+make $(MAKEFLAGS) \
		LCFLAGS="$$(echo $(LCFLAGS))" \
		LLIBS="$$(echo $(LLIBS))" \
		$(LIB)

$(LIB): $(OBJS)
	$(CC) -shared $(LDFLAGS) $(OBJS) $(LLIBS) -o $@

.c.o:
	$(CC) -c $(LCFLAGS) $(CFLAGS) -o $@ $<

clean:
	rm -f $(LIB) $(OBJS)
	+make $(MAKEFLAGS) -C tests clean

tests:
	+make $(MAKEFLAGS) -C tests all

.PHONY: all clean tests
