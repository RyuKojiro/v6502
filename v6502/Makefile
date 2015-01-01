.include "../config.mk"

PROG=	v6502
SRCS=	main.c ../as6502/parser.c log.c cpu.c mem.c ../as6502/linectl.c ../as6502/error.c ../dis6502/reverse.c ../as6502/symbols.c breakpoint.c textmode.c
LIB= -ledit -lcurses

$(PROG): $(SRCS)
	$(CC) $(LIB) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(PROG)

analyze: $(SRCS)
	$(CC) $(LIB) $(INCLUDE) --analyze $(CFLAGS) $(LDFLAGS) $(SRCS) #-o $(PROG)

clean:
	rm -f $(PROG) $(OBJS)
