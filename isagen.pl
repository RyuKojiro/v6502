#!/usr/bin/perl

use strict;
use warnings;

my %instructions;
my %address_modes = (
	"imp"  => "Implied",
	"imm"  => "Immediate",
	"acc"  => "Accumulator",
	"rel"  => "Relative",
	"abs"  => "Absolute",
	"absx" => "Absolute + X",
	"absy" => "Absolute + Y",
	"ind"  => "Indirect",
	"indx" => "Indirect + X",
	"indy" => "Indirect + Y",
	"zpg"  => "Zeropage",
	"zpgx" => "Zeropage + X",
	"zpgy" => "Zeropage + Y",
);
my %suffixes = (
	"imp"  => "",
	"imm"  => "#imm8",
	"acc"  => "A",
	"rel"  => "m8",
	"abs"  => "m16",
	"absx" => "m16, X",
	"absy" => "m16, Y",
	"ind"  => "(m16)",
	"indx" => "(m16, X)",
	"indy" => "(m16), Y",
	"zpg"  => "*m8",
	"zpgx" => "*m8, X",
	"zpgy" => "*m8, Y",
);


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


# Generate the doxygen file
open(my $out, '>', "ISA.dox");
print $out "/**
\\page isa Instruction Set Reference
\\tableofcontents\n";

foreach (sort keys %instructions) {
	my $nmemonic = $_;

	print $out "\\section isa_$nmemonic $instructions{$nmemonic}{'comment'}\n";
	print $out "Opcodes\n";
	print $out "<table><tr><th>Address Mode</th><th>Nmemonic</th><th>Opcode</th></tr>\n";
	foreach (sort keys $instructions{$nmemonic}) {
		if ($_ ne 'comment') {
			print $out "<tr>";
			print $out "<td>$address_modes{$_}</td>";
			print $out "<td>$nmemonic $suffixes{$_}</td>";
			print $out "<td>$instructions{$nmemonic}{$_}</td>";
			print $out "</tr>";
		}
	}
	print $out "</table>\n";
	#print $out "Implementation\n";
}

print $out "*/\n"
