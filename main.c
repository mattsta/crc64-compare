#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
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

/* GCC 4.8 on Linux is dumb */
#ifndef ftello
extern off_t ftello(FILE *stream);
#endif

int memspeed(int size, int iterations) {
    int size_mb = size * 1024 * 1024;
    char *mem = malloc(size_mb);
    float duration_s, syscall_latency = 0;
    int i, warmup = 10000;
    long long istart, istop, duration;
    long long size_mb_time = size * iterations;
    long filter_overhead = 0;

    for (i = 0; i < warmup; i++) {
        long long start, stop;
        start = ustime();
        memset(mem, 0xC0, 1);
        stop = ustime();
        syscall_latency += stop - start;
    }
    syscall_latency /= warmup;
    printf("Average call overhead: %f microseconds\n", syscall_latency);
    filter_overhead = syscall_latency * iterations;

    istart = ustime();
    while(iterations--) {
        memset(mem, 0xC0, size_mb);
    }
    istop = ustime();
    free(mem);

    duration = istop - istart;
    duration_s = (duration - filter_overhead)/1e6;

    printf("Wrote %llu MB in %f seconds for a speed of %f MB/s (%f GB/s)\n",
        size_mb_time, duration_s, size_mb_time/duration_s, (size_mb_time/duration_s/1024));
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
    }

    if (argc == 1) {
        printf("e9c6d914c4b8d9ca == %016llx\n",
            (unsigned long long) crc64(0,(unsigned char*)"123456789",9));
        return 0;
    }

    char *filename = argv[1];
    FILE *fp = fopen(filename, "r");

    if (fseek(fp, 0, SEEK_END) == -1) {
        perror("Can't find file length");
        return 1;
    }
    off_t sz = ftello(fp);
    rewind(fp);
    char *contents = malloc(sz); /* potentially very big */

    if (fread(contents, sz, 1, fp) != 1) {
        perror("Could not read entire file");
        return 1;
    }
    fclose(fp);

    /* The adler crc64 initializes all lookup tables on the first call,
     * so the first call isn't representative of true speed. */
    crc64(0,(unsigned char*)"init",4);

    long long start = ustime();
    unsigned long long result = crc64(0,(unsigned char*)contents,sz);
    long long end = ustime();
    free(contents);

    double total_time_seconds = (end - start)/1e6;
    double size_mb = sz/1024.0/1024.0;
    double speed = size_mb/total_time_seconds;  /* MB per second */

    if (argc > 2) { /* easier parsing for comparisons */
        printf("%016llx:%lf\n", result, speed);
    } else { /* boring human readable results */
        printf("CRC-64 Result: %016llx\n", result);
        printf("Evaluated %lf MB file in %lf seconds.\n", size_mb, total_time_seconds);
        printf("Speed: %lf megabytes per second\n", speed);
    }

    return 0;
}
