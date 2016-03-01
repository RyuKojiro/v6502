# Compilation
INCLUDE+=       -I..
CFLAGS+=		-Wall -Wno-unknown-pragmas -std=c99 -ggdb $(INCLUDE) -D_XOPEN_SOURCE=500

# Installation
DESTDIR?=		/
PREFIX?=		/usr/local/

MANDEST=		$(DESTDIR)/usr/share/man/man1/
BINDEST=		$(DESTDIR)/$(PREFIX)/bin/
LIBDEST=		$(DESTDIR)/$(PREFIX)/lib/
INCDEST=		$(DESTDIR)/$(PREFIX)/include/
