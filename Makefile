CC=gcc
CFLAGS=-Wall -g
SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

build:
	@mkdir -p bin

all: clean build ws kmv hll cmin

ws: 
	-$(CC) $(CFLAGS) src/ws.c -o bin/ws
kmv: 
	-$(CC) $(CFLAGS) src/kmv.c -lm -o bin/kmv
hll: 
	-$(CC) $(CFLAGS) src/hll.c -o bin/hll
cmin: 
	-$(CC) $(CFLAGS) src/cmin.c -o bin/cmin

clean:
	rm -f bin/*
