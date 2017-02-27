include ../config.mk
include ../libvars.mk

PROG=		dis6502
SRCS=		main.c
LIBSRCS=	reverse.c
LDFLAGS+=	-lld6502 -ldis6502 -las6502 -lv6502 -lcurses
OBJS=		$(SRCS:.c=.o)
LIBOBJS=	$(LIBSRCS:.c=.o)
MANPAGE=	dis6502.1
HEADERS=	reverse.h

all: $(PROG)

install: $(PROG) $(LIBDIS6502)
	install $(MANPAGE) $(MANDEST)
	install $(PROG) $(BINDEST)
	install $(LIBDIS6502) $(LIBDEST)
	install -d $(INCDEST)/$(PROG)/
	install $(HEADERS) $(INCDEST)/$(PROG)/

uninstall: $(PROG) $(LIBDIS6502)
	rm -f $(MANDEST)/$(MANPAGE)
	rm -f $(BINDEST)/$(PROG)
	rm -f $(LIBDEST)/$(LIBDIS6502)
	rm -rf $(INCDEST)/$(PROG)/

lib: $(LIBOBJS)
	$(AR) $(ARFLAGS) libdis6502.a $(LIBOBJS)

$(PROG): $(OBJS) $(LIBV6502) $(LIBAS6502) $(LIBDIS6502) $(LIBLD6502)
	$(CC) $(OBJS) -o $(PROG) $(LDFLAGS)

cleanlib:
	rm -f $(LIBDIS6502) $(LIBOBJS)

clean: cleanlib
	rm -f $(PROG) $(OBJS)

include ../libtargets.mk
