include ../config.mk
include ../libvars.mk

PROG=		dis6502
SRCS=		main.c
LDFLAGS+=	-lv6502 -las6502 -ldis6502 -lld6502
OBJS=		$(SRCS:.c=.o)

$(PROG): $(OBJS) $(LIBV6502) $(LIBAS6502) $(LIBDIS6502) $(LIBLD6502)
	$(CC) $(LDFLAGS) $(OBJS) -o $(PROG)

clean:
	rm -f $(PROG) $(OBJS)

include ../libtargets.mk
