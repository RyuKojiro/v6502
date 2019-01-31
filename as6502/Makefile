include ../config.mk
include ../libvars.mk

PROG=		as6502
SRCS=		main.c
LIBSRCS=	linectl.c parser.c codegen.c symbols.c error.c token.c debug.c
LDFLAGS+=	-lld6502 -ldis6502 -las6502 -lv6502 -lcurses
OBJS=		$(SRCS:.c=.o)
LIBOBJS=	$(LIBSRCS:.c=.o)
MANPAGE=	as6502.1
HEADERS=	parser.h codegen.h symbols.h token.h linectl.h color.h

all: $(PROG)

install: $(PROG) $(LIBAS6502)
	install $(MANPAGE) $(MANDEST)
	install $(PROG) $(BINDEST)
	install $(LIBAS6502) $(LIBDEST)
	install -d $(INCDEST)/$(PROG)/
	install $(HEADERS) $(INCDEST)/$(PROG)/

uninstall:
	rm -f $(MANDEST)/$(MANPAGE)
	rm -f $(BINDEST)/$(PROG)
	rm -f $(LIBDEST)/$(LIBAS6502)
	rm -rf $(INCDEST)/$(PROG)/

lib: $(LIBOBJS)
	$(AR) $(ARFLAGS) libas6502.a $(LIBOBJS)

$(PROG): $(OBJS) $(LIBV6502) $(LIBAS6502) $(LIBDIS6502) $(LIBLD6502)
	$(CC) $(OBJS) -o $(PROG) $(LDFLAGS)

cleanlib:
	rm -f $(LIBAS6502) $(LIBOBJS)

clean: cleanlib
	rm -f $(PROG) $(OBJS)

include ../libtargets.mk
