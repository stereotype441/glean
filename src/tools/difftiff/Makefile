include $(GLEAN_ROOT)/make/common.mak

TARGET=difftiff
ifeq ($(PLATFORM), MacOSX)
	LIB=-framework GLUT -framework OpenGL -framework AGL -framework Carbon -ldsurf -llex -limage -lstats -ltimer -ltiff
else
	LIB=-limage -ltiff -lglut -lGLU -lGL -lXmu -lXext -lXi -lX11 $(EXTRALIBS)
endif # MacOSX

include $(GLEAN_ROOT)/make/app.mak
