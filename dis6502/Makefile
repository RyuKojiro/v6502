include ../config.mk
include ../libvars.mk

PROG=		dis6502
SRCS=		main.c
LDFLAGS+=	-lld6502 -ldis6502 -las6502 -lv6502
OBJS=		$(SRCS:.c=.o)

all: $(PROG)

lib: $(LIBDIS6502_OBJS)
	$(AR) $(ARFLAGS) libdis6502.a $(LIBDIS6502_OBJS)

$(PROG): $(OBJS) $(LIBV6502) $(LIBAS6502) $(LIBDIS6502) $(LIBLD6502)
	$(CC) $(OBJS) -o $(PROG) $(LDFLAGS)

cleanlib:
	rm -f $(LIBDIS6502) $(LIBDIS6502_OBJS)

clean: cleanlib
	rm -f $(PROG) $(OBJS)

include ../libtargets.mk
