include $(GLEAN_ROOT)/make/common.mak

TARGET=showvis
ifeq ($(PLATFORM), MacOSX)
	LIB=-framework GLUT -framework OpenGL -framework AGL -framework Carbon -ldsurf -llex -limage -lstats -ltimer -ltiff
else
	LIB=-ldsurf -llex -lGL $(EXTRALIBS)
endif # MacOSX


include $(GLEAN_ROOT)/make/app.mak
