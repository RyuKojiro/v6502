include ../config.mk

PROG=	dis6502
SRCS=	main.c ../as6502/symbols.c ../ld6502/object.c ../dis6502/reverse.c \
	../as6502/parser.c ../v6502/cpu.c ../v6502/mem.c ../as6502/linectl.c \
	../ld6502/flat.c ../as6502/error.c ../ld6502/ines.c
OBJS=	$(SRCS:.c=.o)

$(PROG): $(SRCS)
	$(CC) $(LIB) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(PROG)

clean:
	rm -f $(PROG) $(OBJS)
