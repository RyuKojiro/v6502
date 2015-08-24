LDFLAGS+=			-L../as6502 -L../dis6502 -L../ld6502 -L../v6502

LIBAS6502=			../as6502/libas6502.a
LIBAS6502_SRCS=		../as6502/linectl.c ../as6502/parser.c ../as6502/codegen.c ../as6502/symbols.c ../as6502/error.c ../as6502/token.c
LIBAS6502_OBJS=		$(LIBAS6502_SRCS:.c=.o)

LIBDIS6502=			../dis6502/libdis6502.a
LIBDIS6502_SRCS=	../dis6502/reverse.c
LIBDIS6502_OBJS=	$(LIBDIS6502_SRCS:.c=.o)

LIBLD6502=			../ld6502/libld6502.a
LIBLD6502_SRCS=		../ld6502/object.c ../ld6502/aout.c ../ld6502/flat.c ../ld6502/ines.c
LIBLD6502_OBJS=		$(LIBLD6502_SRCS:.c=.o)

LIBV6502=			../v6502/libv6502.a
LIBV6502_SRCS=		../v6502/cpu.c ../v6502/mem.c ../v6502/cpu.c
LIBV6502_OBJS=		$(LIBV6502_SRCS:.c=.o)
