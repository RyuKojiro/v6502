$(LIBAS6502): $(LIBAS6502_OBJS)
	$(AR) $(ARFLAGS) $@ $(LIBAS6502_OBJS)

$(LIBDIS6502): $(LIBDIS6502_OBJS)
	$(AR) $(ARFLAGS) $@ $(LIBDIS6502_OBJS)

$(LIBLD6502): $(LIBLD6502_OBJS)
	$(AR) $(ARFLAGS) $@ $(LIBLD6502_OBJS)

$(LIBV6502): $(LIBV6502_OBJS)
	$(AR) $(ARFLAGS) $@ $(LIBV6502_OBJS)
