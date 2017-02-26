SUBDIRS=	as6502 dis6502 ld6502 v6502 tests

all clean cleandir depend analyze install uninstall lib:
	@for dir in $(SUBDIRS) ; do			\
		echo "==> $$dir ($@)";			\
		$(MAKE) -C $$dir $@ || exit 1;		\
	done
	@if [ "$@" = "clean" -o "$@" = "cleandir" ] ; then \
		rm -rf $(PACKAGE_DIR) ; \
		rm -f $(PACKAGE_NAME) ; \
	fi

docs:
	$(MAKE) -C kmapgen
	./isagen.pl
	doxygen
