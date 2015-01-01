ASDIR=	../as6502
AS=	$(ASDIR)/as6502

DISDIR=	../dis6502
DIS=	$(DISDIR)/dis6502

VMDIR=	../v6502
VM=	$(VMDIR)/v6502

tests: expectedAssemblyResult circularAssemble #wozfp

# This attempts to assemble the snake code, disassemble the result, assemble the
# disassembler result, and then compare the originally assembled binary, with
# the reassembled binary.
circularAssemble: $(AS) $(DIS)
	$(AS) snake.s
	$(DIS) snake.o > snake2.s
	$(AS) snake2.s
	diff snake.o snake2.o

# This attempts to assemble the snake code, comparing the result with an MD5
# hash of a known-working-good binary.
expectedAssemblyResult: $(AS)
	$(AS) snake.s
	md5 snake.o > snake.tmp.md5
	diff snake.o.md5 snake.tmp.md5

# This simply tries to assemble the Apple II floating point routines originally
# written by Steve Wozniak.
wozfp: $(AS)
	$(AS) wozfp1.s

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
	rm -f *.o
