CC=clang
PROG=	easy6502
SRCS=	main.c video.c ../v6502/cpu.c ../v6502/mem.c
INCLUDE+=	-I../v6502/
LIB= -lcurses
CFLAGS=	-Wall

$(PROG): $(SRCS)
	$(CC) $(LIB) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(PROG)

analyze: $(SRCS)
	$(CC) $(LIB) $(INCLUDE) --analyze $(CFLAGS) $(LDFLAGS) $(SRCS) #-o $(PROG)

clean:
	rm -f $(PROG) $(OBJS)
