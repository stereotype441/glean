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
#    The window system.  Define either __X11__ or __WIN__ or __BEWIN__ or __AGL__.

# The variable PLATFORM sets CONFIG and other variables to common
# default values for various operating systems.  This allows the bulk
# of the GNU-based Makefiles to be shared between operating systems.
# The values currently accepted are "BeOS", "Unix", and "MacOSX".

# Major configuration options:
#PLATFORM:=BeOS
#PLATFORM:=MacOSX
PLATFORM:=Unix

ifeq ($(PLATFORM), Unix)
	CONFIG:=-D__UNIX__ -D__X11__
else
ifeq ($(PLATFORM), BeOS)
	CONFIG:=-D__UNIX__ -D__BEWIN__
else
ifeq ($(PLATFORM), MacOSX)
	CONFIG:=-D__UNIX__ -D__AGL__
else
	Configuration error.  Please set PLATFORM variable appropriately.
endif # MacOSX
endif # BeOS
endif # Unix


# Locations of common commands:
ifeq ($(PLATFORM), Unix)
	AR:=/usr/bin/ar
	CC:=/usr/bin/g++
	INSTALL:=/usr/bin/install
	RANLIB:=/usr/bin/ranlib
	RM:=/bin/rm
	SED:=/bin/sed
	SHELL:=/bin/sh
	MKDIR:=/bin/mkdir
endif # Unix
ifeq ($(PLATFORM), BeOS)
	AR:=/boot/develop/tools/gnupro/bin/ar
	CC:=g++
	INSTALL:=/bin/install
	RANLIB:=/boot/develop/tools/gnupro/bin/ranlib
	RM:=/bin/rm
	SED:=/bin/sed
	SHELL:=/bin/sh
	MKDIR:=/bin/mkdir
endif # BeOS
ifeq ($(PLATFORM), MacOSX)
	AR:=/usr/bin/ar
	CC:=/usr/bin/c++
	INSTALL:=/usr/bin/install
	RANLIB:=/usr/bin/ranlib
	RM:=/bin/rm
	SED:=/usr/bin/sed
	SHELL:=/bin/sh
	MKDIR:=/bin/mkdir
endif # MacOSX

# Locations of useful include and library files:
ifeq ($(PLATFORM), Unix)
	# Note:  Do *not* list standard system #include directories here,
	# because some compilers complain about them.
	XINC:=-I/usr/include/X11
	XLIB:=/usr/X11R6/lib
	GLINC:=#-I/usr/include
	GLLIB:=/usr/lib
	GLUTINC:=#-I/usr/include
	GLUTLIB:=/usr/lib
	TIFFINC:=#-I/usr/include
	TIFFLIB:=/usr/lib
	EXTRALIBS:=
endif # Unix
ifeq ($(PLATFORM), BeOS)
	GLINC:=/boot/develop/headers/be/opengl/
	GLLIB:=/boot/beos/system/lib
	TIFFINC:=/boot/somewhere
	TIFFLIB:=/boot/somewhere
	EXTRALIBS:=
endif # BeOS
ifeq ($(PLATFORM), MacOSX)
	GLLIB:=/usr/lib
	GLUTLIB:=/usr/lib
	XLIB:=/usr/lib
	TIFFINC:=/usr/include
	TIFFLIB:=/usr/lib
endif # MacOSX


# Standard targets and subdirectory handling:
all:     all_here     all_dirs
install: install_here install_dirs
clean:   clean_here   clean_dirs
clobber: clobber_here clobber_dirs
test:    test_here    test_dirs

all_dirs:
	for d in $(DIRS) X; do \
		if [ -d "$$d" ] ; then \
			(cd $$d; $(MAKE) all); \
		fi \
	done
install_dirs:
	for d in $(DIRS) X; do \
		if [ -d "$$d" ] ; then \
			(cd $$d; $(MAKE) install); \
		fi \
	done
clean_dirs:
	for d in $(DIRS) X; do \
		if [ -d "$$d" ] ; then \
			(cd $$d; $(MAKE) clean); \
		fi \
	done
	if [ -d Test ]; then (cd Test; $(MAKE) clean); fi
clobber_dirs:
	for d in $(DIRS) X; do \
		if [ -d "$$d" ] ; then \
			(cd $$d; $(MAKE) clobber); \
		fi \
	done
	if [ -d Test ]; then (cd Test; $(MAKE) clobber); fi
test_dirs:
	for d in $(DIRS) X; do \
		if [ -d "$$d" ] ; then \
			(cd $$d; $(MAKE) test); \
		fi \
	done

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
	-I$(GLEAN_ROOT)/src/libs/dsurf/ \
	-I$(GLEAN_ROOT)/src/libs/image/ \
	-I$(GLEAN_ROOT)/src/libs/lex/ \
	-I$(GLEAN_ROOT)/src/libs/rand/ \
	-I$(GLEAN_ROOT)/src/libs/stats/ \
	-I$(GLEAN_ROOT)/src/libs/timer/ \
	-I$(GLEAN_ROOT)/src/glh/ \
	$(GLINC) \
	$(GLUTINC) \
	$(XINC) \
	$(TIFFINC)
OPT:=		# Optimization options
ifeq ($(PLATFORM), Unix)
_OPT=\
	 -fno-unroll-all-loops \
	$(OPT)
endif # Unix
ifeq ($(PLATFORM), BeOS)
_OPT=\
	-O3 -fno-unroll-all-loops \
	$(OPT)
endif # BeOS
DBG:=-g		# Debugging options
_DBG=\
	$(DBG)
WARN:=		# warning options
ifeq ($(PLATFORM), Unix)
_WARN=\
	-Wall \
	-W \
	$(WARN)
endif # Unix
ifeq ($(PLATFORM), BeOS)
_WARN=\
	-Wall \
	-Wno-multichar -Wno-ctor-dtor-privacy \
	-W \
	$(WARN)
endif # BeOS
ifeq ($(PLATFORM), MacOSX)
_WARN=\
	-Wall \
	-Wextra \
	$(WARN)
endif # BeOS
LIBDIR:=	# -L options for specifying library directories
# Warning: you may need to change the order of the following library
# directories if you have multiple installations of the same libraries.
_LIBDIR=\
	$(LIBDIR) \
	-L$(GLEAN_ROOT)/lib \
	-L$(GLEAN_ROOT)/src/libs/dsurf \
	-L$(GLEAN_ROOT)/src/libs/image \
	-L$(GLEAN_ROOT)/src/libs/lex \
	-L$(GLEAN_ROOT)/src/libs/rand \
	-L$(GLEAN_ROOT)/src/libs/stats \
	-L$(GLEAN_ROOT)/src/libs/timer \
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
