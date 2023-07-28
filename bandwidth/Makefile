CC=gcc
CXX=g++
CFLAGS=
CXXFLAGS=-std=c++17 -Wall -Werror -Wextra -Wshadow
OPTFLAGS=-O3 -DNDEBUG -march=native -mtune=native

all: bw.x stream.x

bw.x: bw.cpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) bw.cpp -o bw.x

stream.x: stream.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -DSTREAM_ARRAY_SIZE=100000000 -DNTIMES=100 stream.c -o stream.x

clean:
	rm -f bw.x stream.x
