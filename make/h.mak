# Rules for building and installing .h files for glean.
# See template.mak for configurable variables and usage information.

include $(GLEAN_ROOT)/make/dep.mak

all_here:

clobber_here: clean_here

test_here:
	@if [ -d Test ]; then (cd Test; $(MAKE) test); fi

# Note:  The tricky test in the following commands ensures correct
#	results if the destination file does not exist.
install_here:
	for t in $(HTARGET); do \
		if [ ! $(HDST)/$$t -nt $$t ]; then \
			$(RM) -f $(HDST)/$$t; \
			$(INSTALL) -m 0444 $$t $(HDST); \
			fi; \
		done
