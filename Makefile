CC=gcc
CFLAGS=-Wall -g
SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

build:
	@mkdir -p bin

all: clean build ws kmv hll 

ws: 
	-$(CC) $(CFLAGS) src/ws.c -o bin/ws
kmv: 
	-$(CC) $(CFLAGS) src/kmv.c -lm -o bin/kmv
hll: 
	-$(CC) $(CFLAGS) src/hll.c -lm -o bin/hll

clean:
	rm -f bin/*
