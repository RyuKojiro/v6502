CC=clang
PROG=	v6502
SRCS=	main.c ../as6502/parser.c core.c cpu.c mem.c ../as6502/linectl.c ../as6502/error.c ../as6502/reverse.c
INCLUDE+=	-I../as6502/ -I../v6502/
#LIB= -lncurses
CFLAGS=	-Wall

$(PROG): $(SRCS)
	$(CC) $(LIB) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(PROG)

analyze: $(SRCS)
	$(CC) $(LIB) $(INCLUDE) --analyze $(CFLAGS) $(LDFLAGS) $(SRCS) #-o $(PROG)

clean:
	rm -f $(PROG) $(OBJS)
