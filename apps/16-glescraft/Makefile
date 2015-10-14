LDLIBS=-lm -lglut -lGL -lGLEW -std=c++0x
CXXFLAGS=-O6 -ffast-math -Wall
all: glescraft
clean:
	rm -f *.o glescraft
glescraft: ../common/shader_utils.o
.PHONY: all clean
