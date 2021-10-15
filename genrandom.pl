#!/usr/bin/env perl

use bytes;
use strict;
use warnings;

use constant KB => 1024;
use constant MB => 1024*1024;
use constant CHUNK_SIZE => 4 * KB;

my $nbytes = (shift || 4) * MB;
my $chunks = int($nbytes / CHUNK_SIZE);

binmode *STDOUT;

for my $i (1 .. $chunks) {
    print join '', map chr(int (rand(0x100))), 1 .. CHUNK_SIZE;
}
