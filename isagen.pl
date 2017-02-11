#!/usr/bin/perl

use strict;
use warnings;

my %instructions;

# Parse cpu.h into a table of instructions
my $cpu_source = "v6502/cpu.h";
open my $f, $cpu_source or die "Unable to read from cpu header";

while (my $line = <$f>) {
	if($line =~ /v6502_opcode_([[:alpha:]]+)_?([[:alpha:]]*)\s*=\s*(0x[[:xdigit:]]{2}),\s*\/{0,2}\s+(.*)/) {
		my $nmemonic = $1;
		my $opcode = $3;
		my $comment = $4;
		my $addressmode = "imp";
		if($2) {
			$addressmode = $2;
		}
		print "$nmemonic, $opcode, $addressmode, $comment\n";

		$instructions{$nmemonic}{$addressmode} = $opcode;
		if($comment) {
			$instructions{$nmemonic}{'comment'} = $comment;
		}
	}
}

foreach (sort keys %instructions) {
	my $x = $_;
	print "$_\n";
	foreach (sort keys $instructions{$x}) {
		print "\t$_ : $instructions{$x}{$_}\n";
	}
}

