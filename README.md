# seq-tools

A set of tools for sequence conversion and processing.
Each tool is dedicated to single task on specific kind of data.
These tools are super-fast, use constant RAM, and scale to huge data sizes.

## Motivation

When faced with a simple sequence-processing task, normally we just throw a grep/sed/awk one-liner,
or a trivial perl/python/ruby script.
In rare cases when more performance is needed,
we may check existing tools such as seqtk.
(Finding and installing a suitable tool and looking up its usage typically takes longer time than writing a script by yourself).

However, every once in a while your project may have unique sequence processing needs:

* A simple transformation of sequence data.
* It has to be extremely fast and scale to massive data.
* It has to integrate smoothly into a pipeline.
* It has to have constant and nearly invisible memory footprint.
* It has to be simple and have no dependencies.
* The data is pre-validated or otherwise under our control.

For such cases you are welcome to try some of these tools.

Note that this repo only has tools that I personally needed myself in the past.
If your required task is not covered, but is sufficiently simple,
and has no available super-fast tool, please post a request in the Issues.

## Features

**Simplicity:** These tools are intentionally very simple.
Each tool is doing one task only in the most natural way.
The logic of each tool is straightforward and understandable.
Crucially, this means that even a non-specialist can understand them,
maintain them, and adapt them to specific needs.

**Simplicity:** (again, because it's important).
These tools don't pull massive libraries or frameworks.
They don't consist of hundreds of interlinked source files.
When they don't work as you expect, you don't have to untangle call chains 20 levels deep.
These tools do the minimum work necessary to accomplish their tasks.

**Reliablility:** These tools are written in standard C language,
and don't depend on external libraries.
Therefore they are likely to continue working with minimal maintenance.

**Documentation:** These tools have clearly documented assumptions and expectations.
File formats and conventions are explicitly documented.
(Please submit an issue if you find something missing).

**Performance:** These tools are very fast and have minimal memory consumption.

## Alternatives

Naturally, please be aware of numerous other toolkits that may or may not better suit your needs:

* [Seqtk](https://github.com/lh3/seqtk)
* [FASTX-Toolkit](http://hannonlab.cshl.edu/fastx_toolkit/)
* [SeqKit](https://github.com/shenwei356/seqkit)
* [seqmagick](https://fhcrc.github.io/seqmagick/)
* [Fasta Utilities](https://github.com/jimhester/fasta_utilities)

## Synopsis

`seq-t2u <in.seq >out.seq` - Convert T to U.

`seq-u2t <in.seq >out.seq` - Convert U to T.

`seq-merge-lines <in.seq >out.seq` - Remove end-of-line characters.

`seq-split-to-lines --line-length 100 <in.seq >out.seq` - Split single-line sequence into lines.
