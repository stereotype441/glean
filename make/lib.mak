# Rules for building and installing libraries (and .h files) for glean.
# See template.mak for configurable variables and usage information.

include $(GLEAN_ROOT)/make/dep.mak

all_here: $(TARGET)

clobber_here: clean_here
	$(RM) -f $(TARGET)

test_here:
	@if [ -d Test ]; then (cd Test; $(MAKE) test); fi

# Note:  The tricky test in the following commands ensures correct
#	results if the destination file does not exist.
install_here: $(TARGET)
	@if [ ! -d $(LIBDST) ]; then $(MKDIR) $(LIBDST); fi
	for t in $(TARGET); do \
		if [ ! $(LIBDST)/$$t -nt $$t ]; then \
			$(RM) -f $(LIBDST)/$$t; \
			$(INSTALL) -m 0444 $$t $(LIBDST); \
			$(RANLIB) $(LIBDST)/$$t; \
			fi; \
		done
	for t in $(HTARGET); do \
		if [ ! $(HDST)/$$t -nt $$t ]; then \
			$(RM) -f $(HDST)/$$t; \
			$(INSTALL) -m 0444 $$t $(HDST); \
			fi; \
		done

$(TARGET): $(OBJ)
	$(RM) -f $@
	$(AR) qc $@ $(OBJ)
