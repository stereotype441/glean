# Makefile for glean

include $(GLEAN_ROOT)/make/common.mak

TARGET=glean

ifeq ($(PLATFORM), Unix)
	LIB=-lstats -ldsurf -llex -limage -ltimer -ltiff -lGLU -lGL -lXmu -lXext -lX11
endif # Unix
ifeq ($(PLATFORM), BeOS)
	LIB=-lstats -ldsurf -llex -limage -ltimer -ltiff -lGL -lbe
endif # BeOS

include $(GLEAN_ROOT)/make/app.mak
