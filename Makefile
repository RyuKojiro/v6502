AS=	as6502
ASDIR=	as6502
VM=	v6502
VMDIR=	v6502
DIS= dis6502
DISDIR= dis6502
LD= ld6502
LDDIR= ld6502
BINS= $(ASDIR)/$(AS) $(VMDIR)/$(VM) $(DISDIR)/$(DIS) $(LDDIR)/$(LD)
INSTALLDIR= /usr/local/bin
all: $(BINS)

$(VMDIR)/$(VM):
	cd $(VMDIR) ; make

$(ASDIR)/$(AS):
	cd $(ASDIR) ; make

$(DISDIR)/$(DIS):
	cd $(DISDIR) ; make

$(LDDIR)/$(LD):
	cd $(LDDIR) ; make

install:
	chmod a+x $(ASDIR)/$(AS); cp $(ASDIR)/$(AS) $(INSTALLDIR)/$(AS)
	chmod a+x $(VMDIR)/$(VM); cp $(VMDIR)/$(VM) $(INSTALLDIR)/$(VM)
	chmod a+x $(LDDIR)/$(LD); cp $(LDDIR)/$(LD) $(INSTALLDIR)/$(LD)
	chmod a+x $(DISDIR)/$(DIS); cp $(DISDIR)/$(DIS) $(INSTALLDIR)/$(DIS)

clean:
	rm -f $(BINS)
