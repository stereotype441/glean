# Rules for building internal tests for glean.
# See template.mak for configurable variables and usage information.

include $(GLEAN_ROOT)/make/dep.mak

all_here: $(TARGET)

clobber_here: clean_here
	$(RM) -f $(TARGET)

test_here: all
	./$(TARGET)

install_here:

$(TARGET): $(OBJ)
	$(CC) $(CONFIG) $(_OPT) $(_DBG) $(_PROF) $(OBJ) $(_LIBDIR) $(_LIB) -o $@
