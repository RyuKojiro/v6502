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
MANDIR= /usr/share/man/man1

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
	install -Cv $(ASDIR)/$(AS) $(INSTALLDIR)/
	cp $(ASDIR)/$(AS).1 $(MANDIR)/ 
	install -Cv $(VMDIR)/$(VM) $(INSTALLDIR)/
	cp $(VMDIR)/$(VM).1 $(MANDIR)/ 
	install -Cv $(LDDIR)/$(LD) $(INSTALLDIR)/
	cp $(LDDIR)/$(LD).1 $(MANDIR)/ 
	install -Cv $(DISDIR)/$(DIS) $(INSTALLDIR)/
	cp $(DISDIR)/$(DIS).1 $(MANDIR)/ 

clean:
	rm -f $(BINS)
