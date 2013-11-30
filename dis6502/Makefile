CC=	clang
PROG=	dis6502
SRCS=	main.c ../as6502/symbols.c ../as6502/object.c ../dis6502/reverse.c \
	../as6502/parser.c ../v6502/cpu.c ../v6502/mem.c ../as6502/linectl.c \
	../as6502/flat.c ../as6502/error.c ../ld6502/ines.c
#LIB=	-lncurses
OBJS=	$(SRCS:.c=.o)
CFLAGS=	-Wall -std=c99
INCLUDE+=	-I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk/usr/include -I../v6502 -I../as6502 -I../ld6502

$(PROG): $(SRCS)
	$(CC) $(LIB) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(PROG)

clean:
	rm -f $(PROG) $(OBJS)
