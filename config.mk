# Compilation
INCLUDE+=       -I..
CFLAGS+=		-Wall -Wno-unknown-pragmas -std=c99 -ggdb $(INCLUDE) -D_XOPEN_SOURCE

# Installation
DESTDIR?=		/
PREFIX?=		/usr/local/

MANDEST=		$(DESTDIR)/$(PREFIX)/man/man1/
BINDEST=		$(DESTDIR)/$(PREFIX)/bin/
LIBDEST=		$(DESTDIR)/$(PREFIX)/lib/
INCDEST=		$(DESTDIR)/$(PREFIX)/include/
