include ../config.mk
include ../libvars.mk

PROG=		ld6502
SRCS=		main.c
LIBSRCS=	object.c aout.c flat.c ines.c
LDFLAGS+=	-lld6502 -ldis6502 -las6502 -lv6502 -lcurses
OBJS=		$(SRCS:.c=.o)
LIBOBJS=	$(LIBSRCS:.c=.o)
MANPAGE=	ld6502.1
HEADERS=	object.h aout.h flat.h ines.h

all: $(PROG)

install: $(PROG) $(LIBLD6502)
	install $(MANPAGE) $(MANDEST)
	install $(PROG) $(BINDEST)
	install $(LIBLD6502) $(LIBDEST)
	install -d $(INCDEST)/$(PROG)/
	install $(HEADERS) $(INCDEST)/$(PROG)/

uninstall:
	rm -f $(MANDEST)/$(MANPAGE)
	rm -f $(BINDEST)/$(PROG)
	rm -f $(LIBDEST)/$(LIBLD6502)
	rm -rf $(INCDEST)/$(PROG)/


lib: $(LIBOBJS)
	$(AR) $(ARFLAGS) libld6502.a $(LIBOBJS)

$(PROG): $(OBJS) $(LIBV6502) $(LIBAS6502) $(LIBDIS6502) $(LIBLD6502)
	$(CC) $(OBJS) -o $(PROG) $(LDFLAGS)

cleanlib:
	rm -f $(LIBLD6502) $(LIBOBJS)

clean: cleanlib
	rm -f $(PROG) $(OBJS)

include ../libtargets.mk
