CC=		clang
PROG=	as6502
SRCS=	main.c labels.c\
		../v6502/parser.c ../v6502/cpu.c ../v6502/mem.c ../v6502/linectl.c
#LIB=	-lncurses
OBJS=	$(patsubst %.c, %.o, $(SRCS))
CFLAGS=	-Wall
INCLUDE+=	-I../v6502

$(PROG): $(SRCS)
	$(CC) $(LIB) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(PROG)

clean:
	rm -f $(PROG) $(OBJS)
