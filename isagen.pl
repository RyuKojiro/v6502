#!/usr/bin/perl

use strict;
use warnings;

# Parse cpu.h into a table of instructions
my $cpu_source = "v6502/cpu.h";
open my $f, $cpu_source or die "Unable to read from cpu header";

while (my $line = <$f>) {
	if($line =~ /v6502_opcode_([[:alpha:]]+)/) {
		print $1 . "\n";
	}
}

