CC=gcc
CPP=g++
CFLAGS=-O3
SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

build:
	@mkdir -p bin

all: clean build qdig gk 

qdig: 
	-$(CC) $(CFLAGS) src/qdig.c -lm -o bin/qdig
gk: 
	-$(CPP) $(CFLAGS) src/gk.cpp -o bin/gk

clean:
	rm -f bin/*
