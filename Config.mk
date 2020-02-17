ifndef PREFIX
    $(error PREFIX is not defined)
endif

EXECUTABLE=curve
LIBDIR=$(PREFIX)/lib
INCLUDEDIR=$(PREFIX)/src

SYSTEM_LIBS=glfw3 freetype2 glm
#OPT=-ggdb
OPT=-DNDEBIG -O2 -ffast-math -Wall -Wextra

CFLAGS=-I$(INCLUDEDIR) `pkg-config --cflags $(SYSTEM_LIBS)`
LDLIBS=`pkg-config --libs $(SYSTEM_LIBS)` -lm -ldl
EXECUTABLE=curve
