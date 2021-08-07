CC=gcc
CFLAGS=-Wall -g
SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

build:
	@mkdir -p bin

all: clean build qdig gk kll

qdig: 
	-$(CC) $(CFLAGS) src/qdig.c -lm -o bin/qdig
gk: 
	-$(CC) $(CFLAGS) src/gk.c -lm -o bin/gk
kll: 
	-$(CC) $(CFLAGS) src/kll.c -lm -o bin/kll

clean:
	rm -f bin/*
