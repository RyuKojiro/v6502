include ../config.mk

PROG=	ld6502
SRCS=	main.c object.c flat.c ../as6502/error.c ../as6502/parser.c ../as6502/linectl.c ../v6502/cpu.c ../v6502/mem.c ines.c ../dis6502/reverse.c ../as6502/symbols.c
OBJS=	$(SRCS:.c=.o)

$(PROG): $(SRCS)
	$(CC) $(LIB) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(PROG)

clean:
	rm -f $(PROG) $(OBJS)
