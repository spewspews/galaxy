LDLIBS=-lSDL2 -lSDL2_gfx -lpthread
CXXFLAGS=-Wall -Wpedantic -Wextra -O0 -std=c++14 -g
CXX=clang++
CC=clang++

nbody: nbody.o ui.o body.o bhtree.o

clean:
	rm -f *.o nbody

.PHONY: clean
