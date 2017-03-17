LDLIBS=-lSDL2 -lSDL2_gfx -lpthread
CXXFLAGS=-Wall -Wpedantic -Wextra -O3 -std=c++11 -g
CXX?=clang++
CC=${CXX}

all: galaxy mkgalaxy

install: all
	cp galaxy mkgalaxy /usr/local/bin

galaxy: galaxy.o ui.o bhtree.o body.o

mkgalaxy: mkgalaxy.o body.o

man:	galaxy.1
	cp $< /usr/local/share/man/man1/$<

clean:
	rm -f *.o galaxy mkgalaxy core.* perf.data

.PHONY: clean all man install
