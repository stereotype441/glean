# Makefile for glean

include $(GLEAN_ROOT)/make/common.mak

TARGET=glean

ifeq ($(PLATFORM), BeOS)
	LIB=-lstats -ldsurf -llex -limage -ltimer -ltiff -lGL -lbe
else
	LIB=-lstats -ldsurf -llex -limage -ltimer -ltiff -lGLU -lGL -lXmu -lXext -lX11
endif

include $(GLEAN_ROOT)/make/app.mak
