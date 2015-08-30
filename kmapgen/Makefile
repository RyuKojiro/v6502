include ../config.mk
include ../libvars.mk

PROG=		kmapgen
SRCS=		main.c
LDFLAGS+=	-ledit -lcurses -ldis6502 -las6502 -lv6502
OBJS=		$(SRCS:.c=.o)

all: $(PROG)

$(PROG): $(OBJS) $(LIBV6502) $(LIBAS6502) $(LIBDIS6502)
	$(CC) $(OBJS) -o $(PROG) $(LDFLAGS)

clean:
	rm -f $(PROG) $(OBJS)

include ../libtargets.mk
