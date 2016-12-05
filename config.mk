# Compilation
INCLUDE+=		-I..
CFLAGS+=		-Wall -Wno-unknown-pragmas -std=c99 $(INCLUDE)
CFLAGS+=		-D_XOPEN_SOURCE # For getopt
CFLAGS+=		-D_XOPEN_SOURCE_EXTENDED # For strndup
CFLAGS+=		-D_GNU_SOURCE # For getline without breaking SIGWINCH (_POSIX_C_SOURCE will break SIGWINCH on many systems)
#CFLAGS+=		-ggdb -O0

# Installation
DESTDIR?=		/
PREFIX?=		/usr/local/

MANDEST=		$(DESTDIR)/usr/share/man/man1/
BINDEST=		$(DESTDIR)/$(PREFIX)/bin/
LIBDEST=		$(DESTDIR)/$(PREFIX)/lib/
INCDEST=		$(DESTDIR)/$(PREFIX)/include/
