LIBPREFIX =
LIBSUFFIX = .so

BIN = autoupnp
LIB = $(LIBPREFIX)autoupnp$(LIBSUFFIX)
DUMMYLIB = $(LIBPREFIX)dummy$(LIBSUFFIX)
OBJS = autoupnp.o notify.o registry.o upnp.o
DUMMYOBJ = dummy.o

WANT_LIBNOTIFY = true

DESTDIR =
PREFIX = /usr
LIBDIRPREFIX = /
DUMMYPREFIX = /usr
LIBDIRNAME = lib

LIBDIR = $(LIBDIRPREFIX)/$(LIBDIRNAME)
DUMMYLIBDIR = $(DUMMYPREFIX)/$(LIBDIRNAME)
BINDIR = $(PREFIX)/bin

DIRUMASK = a+rx
LIBPERMS = a+rx
BINPERMS = a+rx
SUIDPERMS = u+s,g+s

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

$(DUMMYLIB): $(DUMMYOBJ)
	$(CC) -shared $(LDFLAGS) $< -o $@

$(DUMMYOBJ): dummy.c
	$(CC) -c $(CFLAGS) -o $@ $<

dummy.c:
	touch $@

.c.o:
	$(CC) -c $(LCFLAGS) $(CFLAGS) -o $@ $<

clean:
	rm -f $(LIB) $(OBJS) $(DUMMYLIB) $(DUMMYOBJ) dummy.c
	+$(MAKE) -C tests clean

tests:
	+$(MAKE) -C tests all

install: install-common
	sed -i \
		-e 's:\(endswith \)libautoupnp.so:\1$(LIB):g' \
		-e 's:\(set -- \)libautoupnp.so:\1$(LIBDIR)/$(LIB):g' \
		$(DESTDIR)$(BINDIR)/$(BIN)

install-common: $(LIB)
	umask $(DIRUMASK); mkdir -p $(DESTDIR)$(LIBDIR) $(DESTDIR)$(BINDIR)
	cp $(LIB) $(DESTDIR)$(LIBDIR)/
	chmod $(LIBPERMS) $(DESTDIR)$(LIBDIR)/$(LIB)
	cp $(BIN) $(DESTDIR)$(BINDIR)/
	chmod $(BINPERMS) $(DESTDIR)$(BINDIR)/$(BIN)

install-suid: install-common
	chmod $(SUIDPERMS) $(DESTDIR)$(LIBDIR)/$(LIB)
	sed -i -e 's:libautoupnp.so:$(LIB):g' $(DESTDIR)$(BINDIR)/$(BIN)

install-dummy: install-common $(DUMMYLIB)
	umask $(DIRUMASK); mkdir -p $(DESTDIR)$(DUMMYLIBDIR)
	cp $(DUMMYLIB) $(DESTDIR)$(DUMMYLIBDIR)/$(LIB)
	chmod $(LIBPERMS),$(SUIDPERMS) $(DESTDIR)$(DUMMYLIBDIR)/$(LIB)
	sed -i -e 's:libautoupnp.so:$(LIB):g' $(DESTDIR)$(BINDIR)/$(BIN)

.PHONY: all clean dummy tests \
	install install-suid install-dummy install-common
