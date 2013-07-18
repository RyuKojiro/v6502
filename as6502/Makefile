#CC=		clang
PROG=	as6502
SRCS=	main.c symbols.c object.c error.c codegen.c\
	parser.c ../v6502/cpu.c ../v6502/mem.c linectl.c \
	flat.c
#LIB=	-lncurses
OBJS=	$(SRCS:.c=.o)
CFLAGS=	-Wall -std=c99
INCLUDE+=	-I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk/usr/include -I../v6502

$(PROG): $(SRCS)
	$(CC) $(LIB) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(PROG)

clean:
	rm -f $(PROG) $(OBJS)
