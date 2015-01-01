.include "../config.mk"

PROG=	as6502
SRCS=	main.c symbols.c ../ld6502/object.c error.c codegen.c\
	parser.c ../v6502/cpu.c ../v6502/mem.c linectl.c \
	../ld6502/flat.c ../ld6502/ines.c ../dis6502/reverse.c
OBJS=	$(SRCS:.c=.o)

$(PROG): $(SRCS)
	$(CC) $(LIB) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(PROG)

clean:
	rm -f $(PROG) $(OBJS)
