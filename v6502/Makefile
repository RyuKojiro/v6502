include ../config.mk
include ../libvars.mk

PROG=		v6502
SRCS=		main.c log.c breakpoint.c textmode.c
LDFLAGS+=	-ledit -lcurses -lv6502 -las6502 -ldis6502
OBJS=		$(SRCS:.c=.o)

all: $(PROG)

lib: $(LIBV6502)

$(PROG): $(OBJS) $(LIBV6502) $(LIBAS6502) $(LIBDIS6502)
	$(CC) $(LDFLAGS) $(OBJS) -o $(PROG)

analyze: $(SRCS)
	$(CC) $(INCLUDE) --analyze $(CFLAGS) $(LDFLAGS) $(SRCS) #-o $(PROG)

cleanlib:
	rm -f $(LIBV6502) $(LIBV6502_OBJS)

clean: cleanlib
	rm -f $(PROG) $(OBJS)

include ../libtargets.mk
