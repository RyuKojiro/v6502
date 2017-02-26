# Documentation
http://ryukojiro.github.io/v6502

# Building
The entire project is separated into libraries for each of the
Components. You can build each target in its directory, or build its
library with `make lib`. Everything uses portable Makefiles, if you want to
just build and test the entire project, simply run `make` in the top level
directory.

# Testing

Currently, running make in the top level directory will automatically make all
subdirectory targets with the final one being the tests directory. Inside the
tests directory are unit tests and actual test runs of the assembler,
disassembler, interactive VM, and a compiled set of C-based unit tests.

v6502 has been successfully built and tested without modification on the
following platforms:

- x86_64-apple-darwin13.4.0 (clang 6.0, little-endian, 64-bit, GNU make)
- avr (gcc 4.8.1, mixed-endian, 8-bit, GNU make) *libraries only
- i386--netbsdelf (clang 3.4 & gcc 4.5.3, little-endian, 32-bit, BSD make)
- powerpc-apple-darwin9 (gcc 4.0.1, big-endian, 32-bit, GNU make)
- x86_64-unknown-freebsd10.1 (clang 3.4.1, little-endian, 64-bit, BSD make)
- armv6--netbsdelf-eabihf (gcc 4.8.4, little-endian, 32-bit, BSD make)
- x86_64-linux-gnu (gcc 4.7.2, little-endian, 64-bit, GNU make)

There is also an `analyze` target which will run the clang static analyzer
against any library or binary target. This is also makeable from the top level.
