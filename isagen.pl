#!/usr/bin/perl

use strict;
use warnings;

# Parse cpu.h into a table of instructions
my $cpu_source = "v6502/cpu.h";
open my $f, $cpu_source or die "Unable to read from cpu header";

while (my $line = <$f>) {
	if($line =~ /v6502_opcode_([[:alpha:]]+)_?([[:alpha:]]*)\s*=\s*(0x[[:xdigit:]]{2}),\s*\/{0,2}\s+(.*)/) {
		my $mode = "imp";
		if($2) {
			$mode = $2;
		}
		print "$1, $mode, $3";
		if ($4) {
			print " >> $4";
		}
		print "\n";
	}
}

