# Rules for building and installing the null target.  Typically used
# when the directory contains only subdirectories, and has no
# application or library.
# See template.mak for configurable variables and usage information.

include $(GLEAN_ROOT)/make/dep.mak

all_here:

clobber_here: clean_here

test_here:
	@if [ -d Test ]; then (cd Test; $(MAKE) test); fi

install_here:
