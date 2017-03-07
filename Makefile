LDLIBS=-lSDL2 -lSDL2_gfx -lpthread
CXXFLAGS=-Wall -Wpedantic -Wextra -O2 -std=c++14 -g
CXX=clang++
CC=clang++

all: nbody mkgalaxy

install: all
	cp nbody mkgalaxy /usr/local/bin

nbody: nbody.o ui.o bhtree.o galaxy.o

mkgalaxy: mkgalaxy.o galaxy.o

clean:
	rm -f *.o nbody mkgalaxy

.PHONY: clean
