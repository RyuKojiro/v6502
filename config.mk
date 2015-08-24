# Compilation
INCLUDE+=       -I../as6502/ -I../v6502/ -I../dis6502/ -I../ld6502/
CFLAGS+=		-Wall -Wno-unknown-pragmas -std=c99 -ggdb $(INCLUDE) -D_XOPEN_SOURCE

# Installation
DESTDIR?=		/
PREFIX?=		/usr/local/

MANDEST=		$(DESTDIR)/$(PREFIX)/man/man1/
BINDEST=		$(DESTDIR)/$(PREFIX)/bin/
LIBDEST=		$(DESTDIR)/$(PREFIX)/lib/
INCDEST=		$(DESTDIR)/$(PREFIX)/include/
