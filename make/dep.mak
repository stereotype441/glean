# Automatic dependency maintenance rules for glean.
# See template.mak for configurable variables and usage information.

%.dep: %.c
	$(SHELL) -ec '$(CC) $(CONFIG) -MM $(_INC) $< \
		| $(SED) "s/\($*\)\.o[ :]*/\1.o $@: /g" > $@'
%.dep: %.cpp
	$(SHELL) -ec '$(CC) $(CONFIG) -MM $(_INC) $< \
		| $(SED) "s/\($*\)\.o[ :]*/\1.o $@: /g" > $@'
DEP=$(CSRC:.c=.dep) $(CPPSRC:.cpp=.dep)
-include $(DEP) /dev/null
