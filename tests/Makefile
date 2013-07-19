AS=	../as6502
PROG=	test.out
SRCS=	overflow.s
ASFLAGS=	-S

$(PROG): $(AS)
	$(AS) $(ASFLAGS) $(SRCS)

$(AS):
	cd .. ; make

clean:
	rm -f $(PROG)
