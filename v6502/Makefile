include ../config.mk
include ../libvars.mk

PROG=		v6502
SRCS=		main.c log.c breakpoint.c textmode.c debugger.c
LDFLAGS+=	-ledit -lcurses -ldis6502 -las6502 -lv6502
OBJS=		$(SRCS:.c=.o)
MANPAGE=	v6502.1
HEADERS=	textmode.h mem.h cpu.h log.h breakpoint.h debugger.h

all: $(PROG)

install: $(PROG) $(LIBV6502)
	install $(MANPAGE) $(MANDEST)
	install $(PROG) $(BINDEST)
	install $(LIBV6502) $(LIBDEST)
	install -d $(INCDEST)/$(PROG)/
	install $(HEADERS) $(INCDEST)/$(PROG)/

uninstall: $(PROG) $(LIBV6502)
	rm -f $(MANDEST)/$(MANPAGE)
	rm -f $(BINDEST)/$(PROG)
	rm -f $(LIBDEST)/$(LIBV6502)
	rm -rf $(INCDEST)/$(PROG)/

lib: $(LIBV6502_OBJS)
	$(AR) $(ARFLAGS) libv6502.a $(LIBV6502_OBJS)

$(PROG): $(OBJS) $(LIBV6502) $(LIBAS6502) $(LIBDIS6502)
	$(CC) $(OBJS) -o $(PROG) $(LDFLAGS)

cleanlib:
	rm -f $(LIBV6502) $(LIBV6502_OBJS)

clean: cleanlib
	rm -f $(PROG) $(OBJS)

include ../libtargets.mk
