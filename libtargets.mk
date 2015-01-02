$(LIBAS6502):
	make -C ../as6502/ lib

$(LIBDIS6502):
	make -C ../dis6502/ lib

$(LIBLD6502):
	make -C ../ld6502/ lib

$(LIBV6502):
	make -C ../v6502/ lib
