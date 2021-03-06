#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

uint64_t crc64(uint64_t crc, const unsigned char *s, uint64_t l);

static long long ustime(void) {
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long long)tv.tv_sec)*1e6;
    ust += tv.tv_usec;
    return ust;
}

#define DURATION_SEC ((duration - filter_overhead)/1e6)
#define SIZE(i) (size * i)
#define CURRENT_GEEBEES(i) SIZE(i)/DURATION_SEC/1024

int memspeed(int size, int iterations) {
    int size_mb = size * 1024 * 1024;
    char *mem = malloc(size_mb);
    float duration_s, syscall_latency = 0;
    int i, warmup = 10000;
    long long duration = 0;
    long long size_mb_time = size * iterations;
    long filter_overhead = 0;
    long long start, stop;
    unsigned long long bigop_orig, bigop;

    bigop_orig = bigop = 1e9 + 1e9/2;

    printf("Computing average call overhead...\n");
    for (i = 0; i < warmup; i++) {
        start = ustime();
        memset(mem, 0xDA, 1);
        stop = ustime();
        syscall_latency += stop - start;
    }
    syscall_latency /= warmup;
    printf("Average call overhead: %.4f microseconds\n", syscall_latency);
    filter_overhead = syscall_latency * iterations;

    printf("Running %d iterations of writing %d MB of memory\n", iterations, size);
    for (i = 0; i < iterations; i++) {
        if (i && i % 50 == 0)
            printf("Iterations remaining: %d (current throughput: %.2f GB/s)\n", iterations-i, CURRENT_GEEBEES(i));
        start = ustime();
        memset(mem, 0xDA, size_mb);
        stop = ustime();
        duration += stop - start;
    }
    printf("Completed writing.\n");
    free(mem);

    if (duration < filter_overhead) {
        printf("Setting memory took 0 microseconds.  We can't return results.\n");
        printf("Try recompiling without optimizations.\n");
        return EXIT_FAILURE;
    }

    duration_s = DURATION_SEC;
    printf("Wrote %llu MB in %.4f seconds for a memory write speed of %.4f MB/s (%.4f GB/s)\n",
        size_mb_time, duration_s, size_mb_time/duration_s, (size_mb_time/duration_s/1024));

    printf("Now running a quick CPU speed test for %llu iterations...\n", bigop_orig);
    start = ustime();
    while(bigop--) ;
    stop = ustime();

    duration = stop - start;
    duration_s = duration/1e6;
    printf("Iterations completed in %.2f seconds for an execution speed of %.1f million (subtraction) operations per second using one core.\n",
        duration_s, bigop_orig/1e6/duration_s);
    printf("Loop latency: %.2f nanoseconds per iteration.\n", duration*1000/(float)bigop_orig);

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        int size = atoi(argv[1]);
        int iterations = atoi(argv[2]);

        if (size < 0 || iterations < 0) {
            printf("size and iterations must be positive\n");
            return EXIT_FAILURE;
        }
        return memspeed(size, iterations);
    } else  {
        printf("Usage: %s [size of write space in MB] [number of times to write]\n", argv[0]);
    }
}

