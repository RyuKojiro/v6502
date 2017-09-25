include ../config.mk
include ../libvars.mk

SRCS=		main.c ../v6502/log.c
LDFLAGS+=	-lld6502 -ldis6502 -las6502 -lv6502 -lcurses
OBJS=		$(SRCS:.c=.o)

ASDIR=	../as6502
AS=	$(ASDIR)/as6502

DISDIR=	../dis6502
DIS=	$(DISDIR)/dis6502

VMDIR=	../v6502
VM=	$(VMDIR)/v6502

FUZZIN=afl_tests
FUZZOUT=afl_results

all: compiledTests expectedAssemblyResultSnake circularAssemble expectedAssemblyResultTextModeTest wozmon #wozfp

# This requires the afl fuzzer to be present, and does fuzzing of the snake code to find assembler bugs
fuzz_as: $(AS) $(FUZZIN)
	mkdir $(FUZZOUT)
	afl-fuzz -i $(FUZZIN) -o $(FUZZOUT) -M fuzzer01 $(AS) @@

$(FUZZIN):
	mkdir $(FUZZIN)
	cp snake.s wozfp1.s $(FUZZIN)

# This builds and runs the C-based unit test suite. This should test actual
# interaction with the API itself.
compiledTests: compiledTestsExecutable
	./compiledTestsExecutable
   
compiledTestsExecutable: $(OBJS) $(LIBV6502) $(LIBAS6502) $(LIBDIS6502) $(LIBLD6502)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)


# This attempts to assemble the snake code, disassemble the result, assemble the
# disassembler result, and then compare the originally assembled binary, with
# the reassembled binary.
circularAssemble: $(AS) $(DIS)
	$(AS) snake.s
	$(DIS) -s 600 snake.o > snake2.s
	$(AS) snake2.s
	diff snake.o snake2.o

# This attempts to assemble the snake code, comparing the result with an MD5
# hash of a known-working-good binary.
expectedAssemblyResultSnake: $(AS)
	$(AS) snake.s
	cksum snake.o > snake.tmp.crc
	diff snake.o.crc snake.tmp.crc

# This attempts to assemble the snake code, comparing the result with an MD5
# hash of a known-working-good binary.
expectedAssemblyResultTextModeTest: $(AS)
	$(AS) textmode_test.s
	cksum textmode_test.o > textmode_test.tmp.crc
	diff textmode_test.o.crc textmode_test.tmp.crc

# This simply tries to assemble the Apple II floating point routines originally
# written by Steve Wozniak.
wozfp: $(AS)
	$(AS) wozfp1.s

# Similar to the wozfp target, this attempts assembly of the Apple 1 "Woz Monitor"
# originally written by Steve Wozniak, transcribed from the Apple 1 Operation
# Manual. After building the checksum is compared with the known Apple 1 ROM hash.
wozmon: $(AS)
	$(AS) wozmon.s
	cksum wozmon.o > wozmon.tmp.crc
	diff wozmon.o.crc wozmon.tmp.crc

# This attempts to load and run a binary in the VM, currently snake, which won't
# do much with the terminal video.
run: $(VM)
	$(AS) snake.s
	$(VM) snake.o

####  Dependencies  ####

$(VM):
	cd $(VMDIR) ; make

$(AS):
	cd $(ASDIR) ; make

$(DIS):
	cd $(DISDIR) ; make

clean:
	rm -f *.o *.tmp.crc snake.dis.s snake2.s compiledTestsExecutable

lib install uninstall:

.PHONY: all clean lib install uninstall

include ../libtargets.mk
