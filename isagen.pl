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
my %extra_bytes = (
	"imp"  => 0,
	"imm"  => 1,
	"acc"  => 0,
	"rel"  => 1,
	"abs"  => 2,
	"absx" => 2,
	"absy" => 2,
	"ind"  => 2,
	"indx" => 2,
	"indy" => 2,
	"zpg"  => 1,
	"zpgx" => 1,
	"zpgy" => 1,
);
my %implementations = (
	"dec" => "Decrement",
	"dex" => "Decrement",
	"dey" => "Decrement",
	"inc" => "Increment",
	"inx" => "Increment",
	"iny" => "Increment",
	"cmp" => "Compare",
	"cpx" => "Compare",
	"cpy" => "Compare",
);
my %source_implementations;

# Parse cpu.h into a table of instructions
my $cpu_source = "v6502/cpu.h";
open my $f, $cpu_source or die "Unable to read from cpu header";
my $comment;
while (my $line = <$f>) {
	if($line =~ /v6502_opcode_([[:alpha:]]+)_?([[:alpha:]]*)\s*=\s*(0x[[:xdigit:]]{2}),\s*\/{0,2}\s+(.*)/) {
		my $mnemonic = $1;
		my $opcode = $3;
		my $addressmode = "imp";
		if($2) {
			$addressmode = $2;
		} else {
			if ($mnemonic =~ /b[cvpemn][ciesql]/) {
				$addressmode = "rel";
			}
		}
		if($4) {
			$comment = $4;
		}

		$instructions{$mnemonic}{$addressmode} = $opcode;
		if($comment) {
			$instructions{$mnemonic}{'comment'} = $comment;
		}
		undef $comment;
	}
	else {
		if ($line =~ /\/\/\s+(.*)/) {
			$comment = $1;
		}
	}
}

# Parse cpu.c into a table of implementations for those that are not snippet-able
$cpu_source = "v6502/cpu.c";
open $f, $cpu_source or die "Unable to read from cpu source";
my $inside_instruction;
my $lines;
while (my $line = <$f>) {
	if($line =~ /} return;/ and $inside_instruction) {
		if (not $implementations{$inside_instruction}) {
			$source_implementations{$inside_instruction} = $lines;
		}
		undef $inside_instruction;
		undef $lines;
	}

	if($inside_instruction) {
		push @$lines, $line;
	}

	if($line =~ /case v6502_opcode_([[:alpha:]]{3}): {/) {
		$inside_instruction = $1;
	}
}

# Generate the doxygen file
open(my $out, '>', "ISA.dox");
print $out "/**
\\page isa Instruction Set Reference
\\tableofcontents\n";

foreach my $mnemonic (sort keys %instructions) {
	print $out "\\section isa_$mnemonic $instructions{$mnemonic}{'comment'}\n";

	print $out "<table><tr><th>Address Mode</th><th>Mnemonic</th><th>Opcode</th></tr>\n";
	foreach (sort keys $instructions{$mnemonic}) {
		if ($_ ne 'comment') {
			print $out "<tr>";
			print $out "<td>$address_modes{$_}</td>";
			print $out "<td>$mnemonic $suffixes{$_}</td>";
			print $out "<td>$instructions{$mnemonic}{$_}";
		   	if($extra_bytes{$_} >= 1) {
				print $out " 0xLL";
			}
		   	if($extra_bytes{$_} >= 2) {
				print $out " 0xHH";
			}
			print $out "</td>";
			print $out "</tr>";
		}
	}
	print $out "</table>\n";

	print $out "<table><tr><th>Implementation</th></tr>\n";
	print $out "<tr><td>\n";
	if ($source_implementations{$mnemonic}) {
		print $out "\\code\n";
		foreach (@{$source_implementations{$mnemonic}}) {
			print $out "$_";
		}
		print $out "\\endcode\n";
	} else {
		if ($implementations{$mnemonic}) {
			print $out "\\snippet v6502/cpu.c $implementations{$mnemonic}\n";
		} else {
			print $out "\\snippet v6502/cpu.c $mnemonic\n";
		}
	}
	print $out "</td></tr>\n";
	print $out "</table>\n";
	print $out "<hr>\n"; # Self closing tags, and closed hr tags are not allowed by doxygen
}

print $out "*/\n"
