# seq-tools

A set of tools for sequence conversion and processing.
Each tool is dedicated to single task on specific kind of data.
These tools are super-fast, use constant RAM, and scale to huge data sizes.

## Motivation

When faced with a simple sequence-processing task, normally we just throw a grep/sed/awk one-liner,
or a trivial perl/python/ruby script.
In rare cases when more performance is needed, we may check existing tools such as seqtk.
(Finding and installing a suitable tool and looking up its usage typically takes longer time than writing a script by yourself).

However, every once in a while your project may have unique sequence processing needs:

* A simple transformation of sequence data.
* It has to be extremely fast and scale to massive data.
* It has to integrate smoothly into a pipeline.
* It has to have constant and nearly invisible memory footprint.
* The data is pre-validated or otherwise under your control.

For such cases you are welcome to try some of these tools.

Note that this repo only has tools that I personally needed myself in the past.
If your required task is not covered, but is sufficiently simple,
and has no available super-fast tool, please post a request in the Issues.

## Features

**Performance:**
These tools are very fast and have minimal memory consumption.

**Simplicity:**
Each tool performs single task.
These tools are simple, and coded in plain C with no dependencies.

## Alternatives

Naturally, please be aware of numerous other toolkits that may or may not better suit your needs:
[Seqtk](https://github.com/lh3/seqtk),
[FASTX-Toolkit](http://hannonlab.cshl.edu/fastx_toolkit/),
[SeqKit](https://github.com/shenwei356/seqkit),
[seqmagick](https://fhcrc.github.io/seqmagick/),
[Fasta Utilities](https://github.com/jimhester/fasta_utilities).

## Synopsis

`seq-t2u <in.seq >out.seq` - Convert T to U.

`seq-u2t <in.seq >out.seq` - Convert U to T.

`seq-merge-lines <in.seq >out.seq` - Remove end-of-line characters.

`seq-split-to-lines --line-length 100 <in.seq >out.seq` - Split single-line sequence into lines.

`seq-change-case-to-upper <in.seq >out.seq` - Converts sequence to uppercase.

## File formats

**Sequence:** A text file containing only ASCII characters a-z and A-Z.

**Multi-line sequence:** A sequence file where additionally character 0x0A can be used for separating lines.
