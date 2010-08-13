LIB = autoupnp.so
OBJS = autoupnp.o

LCFLAGS = -fPIC
LLIBS = -ldl

all: $(LIB)

$(LIB): $(OBJS)
	$(CC) -shared $(LDFLAGS) $(OBJS) $(LLIBS) -o $@

.c.o:
	$(CC) -c $(LCFLAGS) $(CFLAGS) -o $@ $<

clean:
	rm -f $(LIB) $(OBJS)

.PHONY: all clean
