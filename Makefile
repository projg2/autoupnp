LIB = autoupnp.so
OBJS = autoupnp.o

all: $(LIB)

$(LIB): $(OBJS)
	$(CC) -shared $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f $(LIB) $(OBJS)

.PHONY: all clean
