# wscoder

Bleach/unbleach files

# Inspiration

"[A Tale Of Two Optimisations][1]" by [Greg Foletta][2].

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


[1]: https://articles.foletta.org/post/a-tale-of-two-optimisations/

[2]: https://articles.foletta.org/

[3]: https://github.com/gregfoletta/whitespacer/blob/f86771d10447e1bf57b35ae710637e9e80576d69/README.md#how-performant

[4]: https://ark.intel.com/content/www/us/en/ark/products/39312/intel-core2-duo-processor-t9900-6m-cache-3-06-ghz-1066-mhz-fsb.html

[5]: https://ark.intel.com/content/www/us/en/ark/products/124968/intel-core-i78650u-processor-8m-cache-up-to-4-20-ghz.html
