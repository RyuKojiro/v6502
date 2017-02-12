#!/usr/bin/perl

use strict;
use warnings;

my %instructions;

# Parse cpu.h into a table of instructions
my $cpu_source = "v6502/cpu.h";
open my $f, $cpu_source or die "Unable to read from cpu header";

my $comment;
while (my $line = <$f>) {
	if($line =~ /v6502_opcode_([[:alpha:]]+)_?([[:alpha:]]*)\s*=\s*(0x[[:xdigit:]]{2}),\s*\/{0,2}\s+(.*)/) {
		my $nmemonic = $1;
		my $opcode = $3;
		my $addressmode = "imp";
		if($2) {
			$addressmode = $2;
		}
		if($4) {
			$comment = $4;
		}

		$instructions{$nmemonic}{$addressmode} = $opcode;
		if($comment) {
			$instructions{$nmemonic}{'comment'} = $comment;
		}
		undef $comment;
	}
	else {
		if ($line =~ /\/\/\s+(.*)/) {
			$comment = $1;
		}
	}
}

#foreach (sort keys %instructions) {
#	my $x = $_;
#	print "$_\n";
#	foreach (sort keys $instructions{$x}) {
#		print "\t$_ : $instructions{$x}{$_}\n";
#	}
#}

open(my $out, '>', "ISA.dox");
print $out "/**
\\page isa Instruction Set Reference
\\tableofcontents\n";

foreach (sort keys %instructions) {
	my $nmemonic = $_;

	print $out "\\section isa_$nmemonic $instructions{$nmemonic}{'comment'}\n";
	print $out "Opcodes\n";
	print $out "\\code\n";
	foreach (sort keys $instructions{$nmemonic}) {
		if ($_ ne 'comment') {
			if ($_ eq 'imp') {
				print $out "v6502_opcode_$nmemonic = $instructions{$nmemonic}{$_},\n";
			}
		}
	}
	print $out "\\endcode\n";
	#print $out "Implementation\n";
}

print $out "*/\n"
