CC=gcc
CPP=g++
CFLAGS=-g
SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

build:
	@mkdir -p bin

all: clean build qdig gk baseline

qdig: 
	-$(CPP) $(CFLAGS) src/qdig.cpp -lm -o bin/qdig
gk: 
	-$(CPP) $(CFLAGS) src/gk.cpp -o bin/gk
baseline: 
	-$(CPP) $(CFLAGS) src/baseline.cpp -o bin/baseline

clean:
	rm -f bin/*
