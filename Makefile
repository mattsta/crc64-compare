CFLAGS=-std=c99 -pedantic -g -ggdb3 -O2

.PHONY: all

TARGETS=crc64-redis crc64-adler

all: $(TARGETS)

%: %.c main.c
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

clean:
	rm -f $(TARGETS)
