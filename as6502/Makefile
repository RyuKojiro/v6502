#CC=		clang
PROG=	as6502
SRCS=	main.c symbols.c object.c common.c\
	../v6502/parser.c ../v6502/cpu.c ../v6502/mem.c ../v6502/linectl.c
#LIB=	-lncurses
OBJS=	$(SRCS:.c=.o)
CFLAGS=	-c -Wall -std=c99
INCLUDE+=	-I../v6502

$(PROG): $(OBJS)
	$(CC) $(LIB) $(LDFLAGS) $(OBJS) -o $(PROG)

$(OBJS): $(SRCS)
	$(CC) $(INCLUDE) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROG) $(OBJS)
