include ../config.mk
include ../libvars.mk

PROG=		ld6502
SRCS=		main.c
LDFLAGS+=	-lv6502 -las6502 -ldis6502 -lld6502
OBJS=		$(SRCS:.c=.o)

all: $(PROG)

lib: $(LIBLD6502_OBJS)
	$(AR) $(ARFLAGS) libld6502.a $(LIBLD6502_OBJS)

$(PROG): $(OBJS) $(LIBV6502) $(LIBAS6502) $(LIBDIS6502) $(LIBLD6502)
	$(CC) $(LDFLAGS) $(OBJS) -o $(PROG)

cleanlib:
	rm -f $(LIBLD6502) $(LIBLD6502_OBJS)

clean: cleanlib
	rm -f $(PROG) $(OBJS)

include ../libtargets.mk
