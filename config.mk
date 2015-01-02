INCLUDE+=       -I../as6502/ -I../v6502/ -I../dis6502/ -I../ld6502/
CFLAGS+=	-Wall -Wno-unknown-pragmas -std=c99 -ggdb $(INCLUDE)
