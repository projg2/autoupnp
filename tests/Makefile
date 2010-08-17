TESTS = clean_close fork_close threads

all: $(TESTS)

.c:
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

threads: threads.c
	$(CC) -pthread $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -f $(TESTS)

.PHONY: all clean
