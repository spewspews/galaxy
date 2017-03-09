LDLIBS=-lSDL2 -lSDL2_gfx -lpthread
CXXFLAGS=-Wall -Wpedantic -Wextra -O2 -std=c++14 -g
CXX=clang++
CC=clang++

all: galaxy mkgalaxy

install: all
	cp galaxy mkgalaxy /usr/local/bin

galaxy: galaxy.o ui.o bhtree.o body.o

mkgalaxy: mkgalaxy.o body.o

clean:
	rm -f *.o galaxy mkgalaxy

.PHONY: clean all
