Compare CRC-64 Implementations
==============================

The Redis CRC-64 implementation looked a bit
[too simple](https://github.com/antirez/redis/blob/88c1d9550d198fd7df426b19ea67e9c51c92a811/src/crc64.c)
so I compared it against an
[implementation](http://stackoverflow.com/questions/20562546/how-to-get-crc64-distributed-calculation-use-its-linearity-property)
by Mark Adler and found the Redis implementation is 4x slower than it could be.

(Amusingly, the stackoverflow comment has no votes even though it's
a custom 300 line high throughput C program written just for the answer.)

Notes
-----
The Redis CRC-64 uses custom parameters, so it doesn't generate the same sum as
Mark's CRC-64.

Redis can use the 4x faster CRC by making a new rdb version number then including
the BSD Adler CRC-64 code (while maintaining the old version for backwards
compatability with opening previously saved files).

Or, we can make the Adler code match the Redis parameters and keep all
current behavior and file versions in place.

With the 4x throughput improvement, checksums should be added to all storage formats
by default (even perhaps rolling inside, so we can detect if only certain parts
of a file are corrupted).

Example Differences
-------------------
```haskell
    matt@ununoctium:~/repos/crc64-compare% ./compare.sh /Users/matt/Downloads/Temp\ Keep/Ed\ Sheeran\ -\ Live\ At\ The\ O2\ London\ .mp4
    Filesize: 614M
    Asking crc64-redis to sum "/Users/matt/Downloads/Temp Keep/Ed Sheeran - Live At The O2 London .mp4"...
    Asking crc64-adler to sum "/Users/matt/Downloads/Temp Keep/Ed Sheeran - Live At The O2 London .mp4"...
    Redis CRC-64: e0ff2a57538fa9d4
    Adler CRC-64: afc85640d5e35ba9
    Adler throughput: 1638.609548 MB/s
    Redis throughput: 410.642677 MB/s
    Adler is 3.99x faster than Redis

    matt@ununoctium:~/repos/crc64-compare% ./compare.sh /Users/matt/Downloads/John\ Mayer\ -\ Live\ At\ Austin\ City\ Limits\ PBS\ -\ Full\ Concert-gcdUz12FkdQ.mp4
    Filesize: 731M
    Asking crc64-redis to sum "/Users/matt/Downloads/John Mayer - Live At Austin City Limits PBS - Full Concert-gcdUz12FkdQ.mp4"...
    Asking crc64-adler to sum "/Users/matt/Downloads/John Mayer - Live At Austin City Limits PBS - Full Concert-gcdUz12FkdQ.mp4"...
    Redis CRC-64: ee43263b0a2b6c60
    Adler CRC-64: 0a18e3b48368c9d1
    Adler throughput: 1637.606863 MB/s
    Redis throughput: 401.824009 MB/s
    Adler is 4.07x faster than Redis
```

Bonus
-----
As a bonus, `mem-speed` also lives here.  It takes two arguments: size, in MB, of space to allocate for testing
and the number of iterations to run the test.  For example: `./mem-speed 200 1000` will allocate
200 MB of space and run the test 1000 times.

Example output:
```haskell
matt@ununoctium:~/repos/crc64-compare% ./mem-speed 200 200
Computing average call overhead...
Average call overhead: 0.0566 microseconds
Running 200 iterations of writing 200 MB of memory
Iterations remaining: 150 (current throughput: 14.19 GB/s)
Iterations remaining: 100 (current throughput: 14.95 GB/s)
Iterations remaining: 50 (current throughput: 15.19 GB/s)
Completed writing.
Wrote 40000 MB in 2.5487 seconds for a memory write speed of 15694.1826 MB/s (15.3264 GB/s)
Now running a quick CPU speed test for 1500000000 iterations...
Iterations completed in 2.93 seconds for an execution speed of 511.9 million (subtraction) operations per second using one core.
Loop latency: 1.95 nanoseconds per iteration.
```
