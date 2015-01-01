ASDIR=	../as6502
AS=	$(ASDIR)/as6502
PROG=	wozfp1.o
SRCS=	wozfp1.s
#ASFLAGS=	-S
VM=		v6502
VMDIR=	../v6502

test: $(PROG) $(VM)
	$(VMDIR)/$(VM) $(PROG)

$(PROG): $(AS)
	$(AS) $(ASFLAGS) $(SRCS)

$(VM):
	cd $(VMDIR) ; make

$(AS):
	cd $(ASDIR) ; make

clean:
	rm -f $(PROG)
