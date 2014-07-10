AS=	as6502
ASDIR=	as6502
VM=	v6502
VMDIR=	v6502

all: $(ASDIR)/$(AS) $(VMDIR)/$(VM)

$(VMDIR)/$(VM):
	cd $(VMDIR) ; make

$(ASDIR)/$(AS):
	cd $(ASDIR) ; make

clean:
	rm -f $(ASDIR)/$(AS) $(VMDIR)/$(VM)
