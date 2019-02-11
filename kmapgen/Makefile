include ../config.mk
include ../libvars.mk

PROG=		kmapgen
SRCS=		main.c perf.c time.c ../tests/brute.c
LDFLAGS+=	-ldis6502 -las6502 -lv6502 -ledit -lcurses
CFLAGS+=    -I../tests/ -Os
OBJS=		$(SRCS:.c=.o)
HTML=		kmap.html

all: $(HTML)

$(HTML): $(PROG)
	./$(PROG) > $(HTML)

$(PROG): $(OBJS) $(LIBV6502) $(LIBAS6502) $(LIBDIS6502)
	$(CC) $(OBJS) -o $(PROG) $(LDFLAGS)

clean:
	rm -f $(HTML) $(PROG) $(OBJS)

lib install uninstall:

.PHONY: all clean lib install uninstall

include ../libtargets.mk
