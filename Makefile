LDLIBS=-lSDL2
CXXFLAGS=-Wall -Wpedantic -Wextra -O2 -std=c++14
CXX=clang++
CC=clang++

nbody: nbody.o mouse.o

clean:
	rm -f *.o nbody

.PHONY: clean
