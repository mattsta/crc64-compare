CFLAGS=-std=c99 -pedantic -g -ggdb3

.PHONY: all

TARGETS=crc64-redis crc64-adler mem-speed

all: $(TARGETS)

%: %.c main.c
	$(CC) $(CFLAGS) -O2 -o $@ $^ -lpthread

mem-speed: mem-speed.c
	$(CC) $(CFLAGS) -O0 -o $@ $^

clean:
	rm -f $(TARGETS)
