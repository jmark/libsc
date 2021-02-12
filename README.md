# The SC Library

This is the README file for `libsc`.

The SC Library provides support for parallel scientific applications.

Copyright (C) 2010 The University of Texas System  
Additional copyright (C) 2011 individual authors

`libsc` is written by Carsten Burstedde, Lucas C. Wilcox, Tobin Isaac, and
others.  `libsc` is free software released under the GNU Lesser General
Public Licence version 2.1 (or, at your option, any later version).

The official web page for source code and documentation is
[p4est.org](https://www.p4est.org/).
Please send bug reports and ideas for contribution to `p4est@ins.uni-bonn.de`.
You are also welcome to post issues on
[github](https://www.github.com/cburstedde/libsc.git).
Please see the `CONTRIBUTING` file and
our [coding standards](doc/coding_standards.txt) for details.

## Building `libsc`

The build instructions for `p4est` also apply to standalone builds of `libsc`.

### Autotools

The `autoconf`/`automake`/`libtool` build chain is fully supported.

In a fresh checkout, you may run `./bootstrap` to create the `configure`
script.  This is *not* required for unpacked `tar` archives, or after pulling
fresh code.

We recommend running `configure` with a relative path from an empty build
directory.  Try

    configure --help

for options and switches.  For development with MPI:

    cd empty/build/directory
    ../relative/path/to/configure --enable-mpi --enable-debug \
        CFLAGS="-O0 -g -Wall -Wextra -Wno-unused-parameter"
    make -j8 V=0

The `V=0` environment variable significantly unclutters console output.
