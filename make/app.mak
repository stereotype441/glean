# Standard rules for building, installing, and testing glean applications.
# See template.mak for configurable variables and usage information.

include $(GLEAN_ROOT)/make/dep.mak

all_here: $(TARGET)

clobber_here: clean_here
	$(RM) -f $(TARGET)

test_here:
	@if [ -d Test ]; then (cd Test; $(MAKE) test); fi

install_here: $(TARGET)
	@if [ ! -d $(APPDST) ]; then $(MKDIR) $(APPDST); fi
	(cd $(APPDST); $(RM) -f $(TARGET))
	$(INSTALL) -m 0755 $(TARGET) $(APPDST)

$(TARGET): $(OBJ)
	$(CC) $(CONFIG) $(_OPT) $(_DBG) $(_PROF) $(OBJ) $(_LIBDIR) $(_LIB) -o $@


