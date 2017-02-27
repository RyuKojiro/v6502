ISA=ISA.dox
HTML=html

$(HTML): $(ISA)
	doxygen

$(ISA):
	./isagen.pl

clean:
	rm -rf $(ISA) $(HTML)
