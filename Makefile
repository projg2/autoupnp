LIBPREFIX = lib
LIBSUFFIX = .so

BIN = autoupnp
LIB = $(LIBPREFIX)autoupnp$(LIBSUFFIX)
DUMMYLIB = $(LIBPREFIX)dummy$(LIBSUFFIX)
OBJS = autoupnp.o notify.o registry.o upnp.o

WANT_LIBNOTIFY = true

DESTDIR =
PREFIX = /usr
DUMMYPREFIX = /
LIBDIRNAME = lib

LIBDIR = $(PREFIX)/$(LIBDIRNAME)
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

install: $(LIB)
	umask $(DIRUMASK); mkdir -p $(DESTDIR)$(LIBDIR) $(DESTDIR)$(BINDIR)
	cp $(LIB) $(DESTDIR)$(LIBDIR)/
	chmod $(LIBPERMS) $(DESTDIR)$(LIBDIR)/$(LIB)
	cp $(BIN) $(DESTDIR)$(BINDIR)/
	chmod $(BINPERMS) $(DESTDIR)$(BINDIR)/$(BIN)

install-suid: install
	chmod $(SUIDPERMS) $(DESTDIR)$(LIBDIR)/$(LIB)

install-dummy: install $(DUMMYLIB)
	umask $(DIRUMASK); mkdir -p $(DESTDIR)$(DUMMYLIBDIR)
	cp $(DUMMYLIB) $(DESTDIR)$(DUMMYLIBDIR)/$(LIB)
	chmod $(LIBPERMS),$(SUIDPERMS) $(DESTDIR)$(DUMMYLIBDIR)/$(LIB)

ginstall: $(LIB)
	into $(PREFIX)
	dolib $(LIB)
	dobin $(BIN)

ginstall-suid: ginstall
	fperms $(SUIDPERMS) $(LIBDIR)/$(LIB)

ginstall-dummy: ginstall $(DUMMYLIB)
	into $(DUMMYPREFIX)
	newlib $(DUMMYLIB) $(LIB)
	fperms $(SUIDPERMS) $(DUMMYLIBDIR)/$(LIB)

.PHONY: all clean dummy tests \
	install install-suid install-dummy \
	ginstall ginstall-suid ginstall-dummy
