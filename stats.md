# Some Stats

All timings are with _warm_ OS file cache.

These are not great performance tests. While we shove the bytes to `/dev/null` on output to save on IO on that side, the decoding utility reads 4 times as much data as the encoding utility. That's why the measurements are taken with a warm OS file cache in an attempt to control for that.

## ThinkPad T-400

* CPU: [Core-2 Duo T9600](https://ark.intel.com/content/www/us/en/ark/products/39312/intel-core-2-duo-processor-t9900-6m-cache-3-06-ghz-1066-mhz-fsb.html)
* Memory: 8 GB DDR3
* OS: 64-bit Windows 10 Pro

### Microsoft Compiler

Version: `Microsoft (R) C/C++ Optimizing Compiler Version 19.29.30133 for x64`

#### Encoding

* Input: 268,435,456 pseudo-random bytes
* Output: 1,073,741,824 bytes of whitespace characters

```text
TimeThis :  Command Line :  wse < test.data > NUL
TimeThis :  Elapsed Time :  00:00:02.714
```
#### Decoding

* Input: 1,073,741,824 bytes of whitespace characters
* Output: 268,435,456 pseudo-random bytes identical to contents of `test.data`

```text
TimeThis :  Command Line :  wsd < test.encoded > NUL
TimeThis :  Elapsed Time :  00:00:02.454
```

### Cygwin GCC

Target: `x86_64-pc-cygwin`
Version: `gcc version 11.2.0 (GCC)`

#### Encoding

* Input: 268,435,456 pseudo-random bytes
* Output: 1,073,741,824 bytes of whitespace characters

```text
TimeThis :  Command Line :  wse < test.data > NUL
TimeThis :  Elapsed Time :  00:00:01.128
```
#### Decoding

* Input: 1,073,741,824 bytes of whitespace characters
* Output: 268,435,456 pseudo-random bytes identical to contents of `test.data`

```text
TimeThis :  Command Line :  wsd < test.encoded > NUL
TimeThis :  Elapsed Time :  00:00:05.647
```
I do not know what to make of the differences between `cl` and `gcc`. It seems like `gcc` compiled code encodes 256 MiB and shoves them to oblivion in about 42% of the time it takes the `cl` compiled code. On the other hand, `gcc` compiled code takes more than twice as much time in the decoding task. It seems like if I use `-O2` instead of `-O3`, then encoding performance falls below `cl` compiled code while decoding performance does not change.


