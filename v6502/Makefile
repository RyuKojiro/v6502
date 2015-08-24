include ../config.mk
include ../libvars.mk

PROG=		v6502
SRCS=		main.c log.c breakpoint.c textmode.c
LDFLAGS+=	-ledit -lcurses -ldis6502 -las6502 -lv6502
OBJS=		$(SRCS:.c=.o)

all: $(PROG)

lib: $(LIBV6502_OBJS)
	$(AR) $(ARFLAGS) libv6502.a $(LIBV6502_OBJS)

$(PROG): $(OBJS) $(LIBV6502) $(LIBAS6502) $(LIBDIS6502)
	$(CC) $(OBJS) -o $(PROG) $(LDFLAGS)

cleanlib:
	rm -f $(LIBV6502) $(LIBV6502_OBJS)

clean: cleanlib
	rm -f $(PROG) $(OBJS)

include ../libtargets.mk
