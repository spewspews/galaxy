LDLIBS=-lSDL2 -lSDL2_gfx
CXXFLAGS=-Wall -Wpedantic -Wextra -O0 -std=c++14
CXX=clang++
CC=clang++

nbody: nbody.o mouse.o body.o

clean:
	rm -f *.o nbody

.PHONY: clean
