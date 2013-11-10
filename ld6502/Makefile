CC=	clang
PROG=	ld6502
SRCS=	main.c ../as6502/object.c ../as6502/flat.c ../as6502/error.c
OBJS=	$(SRCS:.c=.o)
CFLAGS=	-Wall -std=c99
INCLUDE+=	-I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk/usr/include -I../as6502

$(PROG): $(SRCS)
	$(CC) $(LIB) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(PROG)

clean:
	rm -f $(PROG) $(OBJS)
