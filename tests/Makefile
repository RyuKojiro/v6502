include ../config.mk
include ../libvars.mk

SRCS=		main.c
LDFLAGS+=	-lld6502 -ldis6502 -las6502 -lv6502
OBJS=		$(SRCS:.c=.o)

ASDIR=	../as6502
AS=	$(ASDIR)/as6502

DISDIR=	../dis6502
DIS=	$(DISDIR)/dis6502

VMDIR=	../v6502
VM=	$(VMDIR)/v6502

all: compiledTests expectedAssemblyResult circularAssemble #wozfp

# This builds and runs the C-based unit test suite. This should test actual
# interaction with the API itself.
compiledTests: $(OBJS) $(LIBV6502) $(LIBAS6502) $(LIBDIS6502) $(LIBLD6502)
	$(CC) $(OBJS) -o $@Executable $(LDFLAGS)
	./$@Executable


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
	rm -f *.o snake.dis.s snake.tmp.md5 snake2.s

include ../libtargets.mk
