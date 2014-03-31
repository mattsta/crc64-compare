CFLAGS=-std=c99 -pedantic -g -ggdb3 -O2

.PHONY: all

TARGETS=crc64-redis crc64-adler

all: $(TARGETS)

%: %.c main.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)
