LDLIBS=-lSDL2 -lSDL2_gfx -lpthread
CXXFLAGS=-Wall -Wpedantic -Wextra -O2 -std=c++14 -g
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
	rm -f *.o galaxy mkgalaxy

.PHONY: clean all man
