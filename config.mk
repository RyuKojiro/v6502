# Compilation
INCLUDE+=       -I..
CFLAGS+=		-Wall -Wno-unknown-pragmas -std=c99 $(INCLUDE) -D_XOPEN_SOURCE=700
#CFLAGS+=		-ggdb -O0

# Installation
DESTDIR?=		/
PREFIX?=		/usr/local/

MANDEST=		$(DESTDIR)/usr/share/man/man1/
BINDEST=		$(DESTDIR)/$(PREFIX)/bin/
LIBDEST=		$(DESTDIR)/$(PREFIX)/lib/
INCDEST=		$(DESTDIR)/$(PREFIX)/include/
