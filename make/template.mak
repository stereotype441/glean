# Template for glean Makefiles.
# When creating a new subdirectory, copy this template into a new Makefile
# and edit the lines below accordingly.

include $(GLEAN_ROOT)/make/common.mak

# Customizable variables with defaults as noted here.  Any or all of these
# may be deleted if the default is acceptable; the actual default values are
# set in common.mak.
CSRC= all *.c files in current directory
CPPSRC= all *.cpp files in current directory
DIRS=all subdirectories with names of the form '*_d' in the current directory
TARGET=app or lib to be built
APPDST=$(GLEAN_ROOT)/bin
LIBDST=$(GLEAN_ROOT)/lib
HTARGET=.h files to be installed (with library targets, or for h.mak below)
HDST=$(GLEAN_ROOT)/include

# Options for compilation:
INC= additional -I options for include-file directories, if any
OPT= additional options for optimization (e.g. -O2), if any
DBG=-g
WARN= warning options
LIBDIR= -L options for additional library directories, if any
LIB= -l options for additional libraries, if any
PROF= various profiling options

# Choose one of these, depending on whether you're building an app, lib,
# .h, tests, or just subdirectories:
include $(GLEAN_ROOT)/make/app.mak
include $(GLEAN_ROOT)/make/lib.mak
include $(GLEAN_ROOT)/make/h.mak
include $(GLEAN_ROOT)/make/test.mak
include $(GLEAN_ROOT)/make/null.mak
