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

all: compiledTests duplicateSymbols assembleFromStdin expectedAssemblyResultSnake circularAssemble expectedAssemblyResultTextModeTest wozmon #wozfp

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

# Assembler and Disassembler automated tests

duplicateSymbols: $(AS)
	@echo "Making sure that duplicate symbols produce an error..."
	! $(AS) double_label.s 2> /dev/null

assembleFromStdin: $(AS)
	@echo "Making sure that assembling from stdin works..."
	$(AS) < snake.s 1> snake3.o 2> /dev/null
	cksum snake3.o | sed 's/snake3.o/snake.o/' > snake3.tmp.crc
	diff snake.o.crc snake3.tmp.crc

circularAssemble: $(AS) $(DIS)
	@echo "Making sure a round trip of assembly -> disassembly -> assembly emits the same binary each time..."
	$(AS) snake.s
	$(DIS) -s 600 snake.o > snake2.s
	$(AS) snake2.s
	diff snake.o snake2.o

expectedAssemblyResultSnake: $(AS)
	@echo "Making sure we can assemble snake.s, and the binary output checksum matches the known correct output..."
	$(AS) snake.s
	cksum snake.o > snake.tmp.crc
	diff snake.o.crc snake.tmp.crc

expectedAssemblyResultTextModeTest: $(AS)
	@echo "Making sure we can assemble textmode_test.s, and the binary output checksum matches the known correct output..."
	$(AS) textmode_test.s
	cksum textmode_test.o > textmode_test.tmp.crc
	diff textmode_test.o.crc textmode_test.tmp.crc

# This simply tries to assemble the Apple II floating point routines originally
# written by Steve Wozniak.
wozfp: $(AS)
	@echo "Making sure we are capable of assembling the Woz floating point routines..."
	$(AS) wozfp1.s

wozmon: $(AS)
	@echo "Making sure we can assemble the Apple 1 Woz Monitor, and the binary output checksum matches the known correct output..."
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
