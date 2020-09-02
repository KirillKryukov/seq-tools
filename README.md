# seq-tools

A toolkit for DNA/RNA sequence conversion and processing.
It's super-fast, uses constant RAM, and scales to huge data sizes.

|Branch      |Status   |
|------------|---------|
|master      | [![Build Status][TravisMasterBadge]][TravisLink] [![Codecov][CodecovMasterBadge]](CodecovLink) |
|develop     | [![Build Status][TravisDevelopBadge]][TravisLink] [![Codecov][CodecovDevelopBadge]](CodecovLink) [![Coverity][CoverityBadge]][CoverityLink] |

[TravisMasterBadge]: https://travis-ci.org/KirillKryukov/seq-tools.svg?branch=master "Continuous Integration test suite"
[TravisDevelopBadge]: https://travis-ci.org/KirillKryukov/seq-tools.svg?branch=develop "Continuous Integration test suite"
[TravisLink]: https://travis-ci.org/KirillKryukov/seq-tools
[CodecovMasterBadge]: https://codecov.io/gh/KirillKryukov/seq-tools/branch/master/graph/badge.svg
[CodecovDevelopBadge]: https://codecov.io/gh/KirillKryukov/seq-tools/branch/develop/graph/badge.svg
[CodecovLink]: https://codecov.io/gh/KirillKryukov/seq-tools
[CoverityBadge]: https://scan.coverity.com/projects/20067/badge.svg?flat=1 "Static code analysis"
[CoverityLink]: https://scan.coverity.com/projects/kirillkryukov-seq-tools

## Installing

Prerequisites: git, gcc, make, diff, perl (diff and perl are only used for test suite).
E.g., to install on Ubuntu: `sudo apt install git gcc make diffutils perl`.
On Mac OS you may have to install Xcode Command Line Tools.

Building from source:

```
git clone https://github.com/KirillKryukov/seq-tools.git
cd seq-tools && make && make test && sudo make install
```

Building from latest unreleased source (for testing purpose only):

```
git clone --branch develop https://github.com/KirillKryukov/seq-tools.git
cd seq-tools && make && make test && sudo make install
```

To install in alternative location, add "prefix=DIR" to the "make install" command. E.g., `sudo make prefix=/usr/local/bio install`

For a staged install, add "DESTDIR=DIR". E.g., `make DESTDIR=/tmp/stage install`

On Windows it can be installed using [Cygwin](https://www.cygwin.com/),
and should be also possible with [WSL](https://docs.microsoft.com/en-us/windows/wsl/install-win10).
In Cygwin drop `sudo`: `cd seq-tools && make && make test && make install`


## Tools

`seq-tools seq-t2u <in.seq >out.seq` - Convert T to U.

`seq-tools seq-u2t <in.seq >out.seq` - Convert U to T.

`seq-tools seq-change-case-to-upper <in.seq >out.seq` - Converts sequence to uppercase.
Removes soft mask from sequence.

`seq-tools seq-merge-lines <in.mseq >out.seq` - Remove end-of-line characters.

`seq-tools seq-split-to-lines --line-length 100 <in.seq >out.mseq` - Split single-line sequence into lines.

`seq-tools seq-soft-mask-extract --mask out.mask <in.seq >out.seq` - Separates mask (positions of lower case characters) from sequence.
Outputs sequence without mask (all in upper case).

`seq-tools seq-soft-mask-add --mask in.mask <in.seq >out.seq` - Soft-masks sequence (changes to lowercase) according to in.mask.

`seq-tools seq-hard-mask-extract --mask out.mask <in.seq >out.seq` - Removes hard mask ("N") from sequence.
Stores mask in a file, outputs sequence without mask (shorter sequence).
Only capital "N" is recognized as hard mask.

`seq-tools seq-hard-mask-add --mask in.mask <in.seq >out.seq` - Adds hard mask (stretches of "N") according to in.mask.
Note: Don't use with untrusted mask file, as a malicious mask can cause the output to exceed your storage capacity.

`seq-tools seq-iupac-extract --iupac out.iupac <in.seq >out.seq` - Removes ambiguous IUPAC nucleotide codes from sequence.
Removes capital characters R, Y, S, W, K, M, B, D, H, V (not N), and stores their locations in the "out.iupac" file.

`seq-tools seq-iupac-add --iupac in.iupac <in.seq >out.seq` - Adds back IUPAC codes,
previously removed with "seq-tools seq-iupac-extract" command.

`seq-tools fasta-unwrap-lines <in.fasta >out.fasta` - Convert each sequence to single line.


## File formats

**Sequence (.seq):** A text file containing only ASCII characters a-z and A-Z.

**Multi-line sequence (.mseq):** A sequence file where additionally character 0x0A can be used for separating lines.

**Mask (.mask):** A binary file storing 64-bit unsigned integer numbers (in platform native endianness).
Each number is the length of a continuous sequence interval with the same mask.
The first interval is always non-masked. If the sequence starts masked, then the first interval length is 0.
Examples: sequence "AaaNAA" has soft mask intervals 1,2,3, and hard mask intervals 3,1,2.

**IUPAC (.iupac):** A binary file that stores locations of ambiguous IUPAC codes (except N).
The file stores a sequence of 9 byte records.
Each records consists of a number (64-bit unsigned, platform-native endianness) and a character.
The meaning is: Skip this many bytes in the input, then add this character.

**FASTA** A text file containing newline-separated records. Each record includes: '>', name, end-of-line character, data.
Data may occupy multiple lines.


## Other tools

Naturally, please be aware of numerous other toolkits that may or may not better suit your needs:
[Seqtk](https://github.com/lh3/seqtk),
[FASTX-Toolkit](http://hannonlab.cshl.edu/fastx_toolkit/),
[SeqKit](https://github.com/shenwei356/seqkit),
[seqmagick](https://fhcrc.github.io/seqmagick/),
[Fasta Utilities](https://github.com/jimhester/fasta_utilities).
