PROG=	ld6502
SRCS=	main.c object.c flat.c ../as6502/error.c ../as6502/parser.c ../as6502/linectl.c ../v6502/cpu.c ../v6502/mem.c ines.c ../dis6502/reverse.c ../as6502/symbols.c
OBJS=	$(SRCS:.c=.o)
CFLAGS=	-Wall -std=c99
INCLUDE+=	-I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk/usr/include -I../as6502 -I../v6502 -I../dis6502 -I.

$(PROG): $(SRCS)
	$(CC) $(LIB) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(PROG)

clean:
	rm -f $(PROG) $(OBJS)
