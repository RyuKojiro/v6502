$(LIBAS6502):
	make -C $(V6502_PREFIX)/as6502/ lib

$(LIBDIS6502):
	make -C $(V6502_PREFIX)/dis6502/ lib

$(LIBLD6502):
	make -C $(V6502_PREFIX)/ld6502/ lib

$(LIBV6502):
	make -C $(V6502_PREFIX)/v6502/ lib

analyze: $(SRCS) $(LIBSRCS)
	$(CC) $(INCLUDE) --analyze -Weverything $(CFLAGS) $(SRCS) $(LIBSRCS)
