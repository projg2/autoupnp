LIB = autoupnp.so
OBJS = autoupnp.o

LCFLAGS = -fPIC

all: $(LIB)

$(LIB): $(OBJS)
	$(CC) -shared $(LDFLAGS) $(OBJS) -o $@

.c.o:
	$(CC) -c $(LCFLAGS) $(CFLAGS) -o $@ $<

clean:
	rm -f $(LIB) $(OBJS)

.PHONY: all clean
