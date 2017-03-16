LDLIBS=-lSDL2 -lSDL2_gfx -lpthread
CXXFLAGS=-Wall -Wpedantic -Wextra -O3 -std=c++11 -g
CXX=clang++
CC=clang++

all: galaxy mkgalaxy

install: all
	cp galaxy mkgalaxy /usr/local/bin

galaxy: galaxy.o ui.o bhtree.o body.o

mkgalaxy: mkgalaxy.o body.o

man:	galaxy.1
	cp $< /usr/local/share/man/man1/$<

clean:
	rm -f *.o galaxy mkgalaxy core.*

.PHONY: clean all man install
