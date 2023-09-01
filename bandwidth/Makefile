CC=gcc
CFLAGS=-Wall -Werror
OPTFLAGS=-O3 -DNDEBUG -march=native -mtune=native -fopenmp

all: bw.x stream.x

bw.x: bw.c
	$(CC) $(CFLAGS) $(OPTFLAGS) bw.c -o bw.x -lm

stream.x: stream.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -DSTREAM_ARRAY_SIZE=100000000 -DNTIMES=100 stream.c -o stream.x

clean:
	rm -f bw.x stream.x
