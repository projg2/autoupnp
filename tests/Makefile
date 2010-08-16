TESTS = clean_close fork_close

all: $(TESTS)

.c:
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -f $(TESTS)

.PHONY: all clean
