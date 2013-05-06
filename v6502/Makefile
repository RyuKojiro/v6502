CC=clang
PROG=	v6502
SRCS=	main.c parser.c core.c cpu.c mem.c linectl.c
#LIB= -lncurses
CFLAGS=	-Wall -Wswitch-enum

$(PROG): $(SRCS)
	$(CC) $(LIB) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(PROG)

analyze: $(SRCS)
	$(CC) $(LIB) $(INCLUDE) --analyze $(CFLAGS) $(LDFLAGS) $(SRCS) #-o $(PROG)

clean:
	rm -f $(PROG) $(OBJS)
