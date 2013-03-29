CC=clang
PROG=	v6502
SRCS=	main.c core.c cpu.c mem.c
#LIB= -lncurses
OBJS=	$(patsubst %.c, %.o, $(SRCS))

$(PROG):
	$(CC) $(LIB) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(PROG) $(OBJS)

clean:
	rm -f $(PROG) $(OBJS)
