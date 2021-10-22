# wscoder

Bleach/unbleach files

# Inspiration

"[A Tale Of Two Optimisations][1]" by [Greg Foletta][2].

See also [my blog post][6] about my thought process.

# Performance

> on \[Greg's\] Intel [i7-8650U][5] laptop, running over a file that's cached in
> memory and outputting to /dev/null, the encoding / decoding process runs at
> 258MiB/s.

In comparison, on my ancient laptop with a [Core 2 Duo T9900][4], I get:

```text
C:\...\wscoder (speedup-attempts) > timethis "wse < test.data | wsd > NUL"

TimeThis :  Command Line :  wse < test.data | wsd > NUL
TimeThis :  Elapsed Time :  00:00:02.831
```

`test.data` is a 256 MiB file containing random bytes. In the pipeline above, `wse` reads 256 MiB and feeds 1 GiB to `wsd` which writes 256 MB to `NUL` (Windows' equivalent of `/dev/null`). I am not quite sure how to measure that, but, let's conservatively say the pipeline is shuffling 1.5 GB of data in 2.831 seconds. That corresponds to a throughput of about 543 MiB/s.

With a warm cache, I get

```text
C:\...\wscoder (speedup-attempts) > timethis "wse < test.data > NUL"

TimeThis :  Command Line :  wse < test.data > NUL
TimeThis :  Elapsed Time :  00:00:02.187
```

when encoding. This reads 256 MiB and writes 1,024 MiB to `NUL` for a throughput of approximately 585 MiB.

Conversely, when decoding with a warm cache:

```text
C:\Users\sinan\src\wscoder (speedup-attempts) > timethis "wsd < test.encoded > NUL"

TimeThis :  Command Line :  wsd < test.encoded > NUL
TimeThis :  Elapsed Time :  00:00:01.852
```

This reads 1,024 MiB and writes 256 MiB to `NUL` for a throughput of approximately 691 MiB/s.

## Threading

One way to improve performance is to take advantage of multiple cores by partitioning the buffers. I decided to give that a shot. To my dismay, I found out the most recent version of Visual Studio does not yet support the optional [C11 threading library][7], so I converted both the encoder and decoder to Franken-C++.

I chose to partition the parts of the buffers processed by each thread in an interleaved manner rather than partitioning into blocks because I assumed (but did not verify) that this would reduce cache trashing.

With two threads on the T9900, both encoding and decoding speed improved:

```text
TimeThis :  Command Line :  wse < test.data > NUL
TimeThis :  Elapsed Time :  00:00:01.404
```

This represents approximately 35% improvement in time and 55% improvement in encoding throughput.

```text
TimeThis :  Command Line :  wsd < test.encoded > NUL
TimeThis :  Elapsed Time :  00:00:01.505
```

This corresponds to about 19% improvement in time and 23% improvement in throughtput. Finally, looking at the round-trip pipeline:

```text
TimeThis :  Command Line :  wse < test.data | wsd >NUL
TimeThis :  Elapsed Time :  00:00:03.009
```
we see that it now executes about 6% slower presumably because we are running four threads on a dual-core machine. Regardless, with an encoder/decoder combination, the common use case is *NOT* to run a round-trip pipeline, so I am OK with that.

Eventually, decided the [a more straightforward optimization][8] mentioned on HN madethe most sense and I incorporated that along with the threads. With both in place, on the same T9900, I get:

```text
TimeThis :  Command Line :  wse < test.data > NUL
TimeThis :  Elapsed Time :  00:00:00.933
```
and

```text
TimeThis :  Command Line :  wsd < test.encoded > NUL
TimeThis :  Elapsed Time :  00:00:01.457
```

Roughly, these correspond to 1.6 GiB/s encoding and 1 GiB/s decoding performance.

[1]: https://articles.foletta.org/post/a-tale-of-two-optimisations/

[2]: https://articles.foletta.org/

[3]: https://github.com/gregfoletta/whitespacer/blob/f86771d10447e1bf57b35ae710637e9e80576d69/README.md#how-performant

[4]: https://ark.intel.com/content/www/us/en/ark/products/39312/intel-core2-duo-processor-t9900-6m-cache-3-06-ghz-1066-mhz-fsb.html

[5]: https://ark.intel.com/content/www/us/en/ark/products/124968/intel-core-i78650u-processor-8m-cache-up-to-4-20-ghz.html

[6]: https://www.nu42.com/2021/10/another-optimization-tale.html

[7]: https://devblogs.microsoft.com/cppblog/c11-and-c17-standard-support-arriving-in-msvc/

[8]: https://news.ycombinator.com/item?id=28859877
