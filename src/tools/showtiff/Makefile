include $(GLEAN_ROOT)/make/common.mak

TARGET=showtiff

ifeq ($(PLATFORM), BeOS)
	LIB=-limage -ltiff -lglut -lGL
else
	LIB=-limage -ltiff -lglut -lGLU -lGL -lXmu -lXext -lXi -lX11
endif

include $(GLEAN_ROOT)/make/app.mak
