include $(GLEAN_ROOT)/make/common.mak

TARGET=showtiff

ifeq ($(PLATFORM), Unix)
	LIB=-limage -ltiff -lglut -lGLU -lGL -lXmu -lXext -lXi -lX11 $(EXTRALIBS)
endif # Unix
ifeq ($(PLATFORM), BeOS)
	LIB=-limage -ltiff -lglut -lGL $(EXTRALIBS)
endif # BeOS

include $(GLEAN_ROOT)/make/app.mak
