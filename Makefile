LIBPREFIX = lib
LIBSUFFIX = .so

LIB = $(LIBPREFIX)autoupnp$(LIBSUFFIX)
DUMMYLIB = $(LIBPREFIX)dummy$(LIBSUFFIX)
OBJS = autoupnp.o notify.o registry.o upnp.o

WANT_LIBNOTIFY = true

LCFLAGS = -fPIC \
	$$($(WANT_LIBNOTIFY) && pkg-config --cflags libnotify && printf '%s' '-DHAVE_LIBNOTIFY')
# pthread-stubs may be required or may be not... try to use it anyway.
LLIBS = -ldl -lminiupnpc \
	$$($(WANT_LIBNOTIFY) && pkg-config --libs libnotify) \
	$$(pkg-config --libs pthread-stubs 2>/dev/null)

all:
	+$(MAKE) \
		LCFLAGS="$$(echo $(LCFLAGS))" \
		LLIBS="$$(echo $(LLIBS))" \
		$(LIB)

$(LIB): $(OBJS)
	$(CC) -shared $(LDFLAGS) $(OBJS) $(LLIBS) -o $@

dummy: $(DUMMYLIB)

$(DUMMYLIB): dummy.o
	$(CC) -shared $(LDFLAGS) $< -o $@

dummy.o: dummy.c
	$(CC) -c $(CFLAGS) -o $@ $<

dummy.c:
	touch $@

.c.o:
	$(CC) -c $(LCFLAGS) $(CFLAGS) -o $@ $<

clean:
	rm -f $(LIB) $(OBJS) $(DUMMYLIB) dummy.o dummy.c
	+$(MAKE) -C tests clean

tests:
	+$(MAKE) -C tests all

.PHONY: all clean dummy tests
