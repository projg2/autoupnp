LIB = autoupnp.so
OBJS = autoupnp.o notify.o registry.o upnp.o

WANT_LIBNOTIFY = true

LCFLAGS = -fPIC $$($(WANT_LIBNOTIFY) && pkg-config --cflags libnotify && printf '%s' '-DHAVE_LIBNOTIFY')
LLIBS = -ldl -lminiupnpc $$($(WANT_LIBNOTIFY) && pkg-config --libs libnotify)

all: $(LIB)

$(LIB): $(OBJS)
	$(CC) -shared $(LDFLAGS) $(OBJS) $(LLIBS) -o $@

.c.o:
	$(CC) -c $(LCFLAGS) $(CFLAGS) -o $@ $<

clean:
	rm -f $(LIB) $(OBJS)

.PHONY: all clean
