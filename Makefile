CC=gcc
CPP=g++
CFLAGS=-g
SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

build:
	@mkdir -p bin

all: clean build qdig gk 

qdig: 
	-$(CPP) $(CFLAGS) src/qdig.cpp -lm -o bin/qdig
gk: 
	-$(CPP) $(CFLAGS) src/gk.cpp -o bin/gk

clean:
	rm -f bin/*
