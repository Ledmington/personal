CC=gcc
CFLAGS=--std=c99 -Wall -Wextra -Wpedantic -Werror -fno-exceptions
DEBUG_FLAGS=-O0 -g -fsanitize=address,undefined
OPT_FLAGS=-O3 -g -DNDEBUG -march=native -mtune=native

all: debug

debug:
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) main.c vector.c bit_stream.c -o main.x

release:
	$(CC) $(CFLAGS) $(OPT_FLAGS) main.c vector.c bit_stream.c -o main.x

clean:
	rm -f *.o main.x
