ASDIR=	../as6502
AS=	$(ASDIR)/as6502
DISDIR=	../dis6502
DIS=	$(DISDIR)/dis6502

PROG=	wozfp1.o
SRCS=	wozfp1.s

#ASFLAGS=	-S

VM=	v6502
VMDIR=	../v6502

tests: circularAssemble expectedAssemblyResult

circularAssemble: $(AS) $(DIS)
	$(AS) snake.s
	$(DIS) snake.o > snake2.s
	$(AS) snake2.s
	diff snake.o snake2.o

expectedAssemblyResult:
	$(AS) snake.s
	md5 snake.o > snake.tmp.md5
	diff snake.o.md5 snake.tmp.md5

run: $(PROG) $(VM)
	$(VMDIR)/$(VM) $(PROG)

$(PROG): $(AS)
	$(AS) $(ASFLAGS) $(SRCS)

$(VM):
	cd $(VMDIR) ; make

$(AS):
	cd $(ASDIR) ; make

$(DIS):
	cd $(DISDIR) ; make

clean:
	rm -f $(PROG)
