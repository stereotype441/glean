# Common rules for building, installing, and testing glean software. 
# Variables set here apply to all glean builds unless overridden by a
# local Makefile.  See template.mak for usage information and for the
# variables that are normally changeable in a local Makefile.  In this
# file, you may find it useful to change the values of CC, INC, OPT,
# DBG, WARN, LIBDIR, and PROF, since those variables apply to nearly
# all compilations.

# The variable CONFIG controls major configuration options.  Currently
# there are two options to be set:
#    The OS (for file operations, etc.).  Define either __UNIX__ or __MS__.
#    The window system.  Define either __X11__ or __WIN__ or __BEWIN__.

# Major configuration options:

#

#PLATFORM=BeOS
PLATFORM=Unix

ifeq ($(PLATFORM), BeOS)
	CONFIG:=-D__UNIX__ -D__BEWIN__
	AR:=/boot/develop/tools/gnupro/bin/ar
	INSTALL:=/bin/install
	GLINC:=/boot/develop/headers/be/opengl/
	GLLIB:=/boot/beos/system/lib
	RANLIB:=/boot/develop/tools/gnupro/bin/ranlib
	TIFFINC:=/boot/somewhere
	TIFFLIB:=/boot/somewhere
else
	CONFIG:=-D__UNIX__ -D__X11__
	AR:=/usr/bin/ar
	INSTALL:=/usr/bin/install
	XINC:=/usr/include/X11
	XLIB:=/usr/X11R6/lib
	GLINC:=/usr/local/include
	GLLIB:=/usr/local/lib
	GLUTINC:=/usr/local/include
	GLUTLIB:=/usr/local/lib
	TIFFINC:=/usr/include
	TIFFLIB:=/usr/lib
	RANLIB:=/usr/bin/ranlib
endif


# Locations of common commands:
SHELL:=/bin/sh
CC:=g++
RM:=/bin/rm
SED:=/bin/sed

# Locations of useful include and library files:


# Standard targets and subdirectory handling:
all:     all_here     all_dirs
install: install_here install_dirs
clean:   clean_here   clean_dirs
clobber: clobber_here clobber_dirs
test:    test_here    test_dirs

all_dirs:
	@for d in $(DIRS); do (cd $$d; $(MAKE) all); done
install_dirs:
	@for d in $(DIRS); do (cd $$d; $(MAKE) install); done
clean_dirs:
	@for d in $(DIRS); do (cd $$d; $(MAKE) clean); done
	@if [ -d Test ]; then (cd Test; $(MAKE) clean); fi
clobber_dirs:
	@for d in $(DIRS); do (cd $$d; $(MAKE) clobber); done
	@if [ -d Test ]; then (cd Test; $(MAKE) clobber); fi
test_dirs:
	@for d in $(DIRS); do (cd $$d; $(MAKE) test); done

clean_here:
	$(RM) -f *.o *.dep *.bak *~

# Default lists of files and subdirectories (may be overridden):
CSRC=$(wildcard *.c)
CPPSRC=$(wildcard *.cpp)
DEP=$(wildcard *.dep)
OBJ=$(CSRC:.c=.o) $(CPPSRC:.cpp=.o)
DIRS=$(wildcard *_d)
TARGET=
APPDST=$(GLEAN_ROOT)/bin
LIBDST=$(GLEAN_ROOT)/lib
HTARGET=
HDST=$(GLEAN_ROOT)/include

# Compilation options:
INC:=		# -I options for local include directories
# Warning: you may need to change the order of the following header-file
# directories if you have multiple installations of the same headers.
_INC=\
	$(INC) \
	-I$(GLEAN_ROOT)/include \
	-I$(GLINC) \
	-I$(GLUTINC) \
	-I$(XINC) \
	-I$(TIFFINC)
OPT:=		# Optimization options
_OPT=\
	-march=pentiumpro \
	-O3 -fno-unroll-all-loops -fnonnull-objects \
	$(OPT)
DBG:=-g		# Debugging options
_DBG=\
	$(DBG)
WARN:=		# warning options /* BeOS requres -Wno-multichar -Wno-ctor-dtor-privacy \ */
_WARN=\
	-Wall \
	-Wno-multichar -Wno-ctor-dtor-privacy \
	-W \
	$(WARN)
LIBDIR:=	# -L options for specifying library directories
# Warning: you may need to change the order of the following library
# directories if you have multiple installations of the same libraries.
_LIBDIR=\
	$(LIBDIR) \
	-L$(GLEAN_ROOT)/lib \
	-L$(GLLIB) \
	-L$(GLUTLIB) \
	-L$(XLIB) \
	-L$(TIFFLIB)
LIB:=		# -l options specifying libraries for linking apps
_LIB=\
	$(LIB)
PROF:=		# profiling options
_PROF=\
	$(PROF)

%.o: %.c
	$(CC) -c $(CONFIG) $(_INC) $(_OPT) $(_DBG) $(_WARN) $(_PROF) $< -o $@
%.o: %.cpp
	$(CC) -c $(CONFIG) $(_INC) $(_OPT) $(_DBG) $(_WARN) $(_PROF) $< -o $@
