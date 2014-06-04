AS=		as6502
ASDIR=	../as6502
PROG=	overflow.o
SRCS=	overflow.s
ASFLAGS=	-S
VM=		v6502
VMDIR=	../v6502

test: $(PROG) $(VM)
	$(VMDIR)/$(VM) $(PROG)

$(PROG): $(AS)
	$(ASDIR)/$(AS) $(ASFLAGS) $(SRCS)

$(VM):
	cd $(VMDIR) ; make

$(AS):
	cd $(ASDIR) ; make

clean:
	rm -f $(PROG)
