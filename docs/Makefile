ISA=ISA.dox
HTML=html
KMAP=../kmapgen/kmap.html

all: $(HTML)

$(HTML): $(ISA) $(KMAP) Doxyfile
	doxygen

$(ISA):
	./isagen.pl

$(KMAP):
	$(MAKE) -C ../kmapgen/ kmap.html

clean:
	rm -rf $(ISA) $(HTML)

lib install uninstall analyze:

.PHONY: all clean lib install uninstall analyze
