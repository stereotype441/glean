# Makefile for glean

include $(GLEAN_ROOT)/make/common.mak

TARGET=glean

ifeq ($(PLATFORM), Unix)
	LIB=-lstats -ldsurf -llex -limage -ltimer -ltiff -lGLU -lGL -lXmu -lXext -lX11 $(EXTRALIBS)
endif # Unix
ifeq ($(PLATFORM), BeOS)
	LIB=-lstats -ldsurf -llex -limage -ltimer -ltiff -lGL -lbe $(EXTRALIBS)
endif # BeOS

include $(GLEAN_ROOT)/make/app.mak
