CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c99
OPTFLAGS=-Ofast -march=native -mtune=native -finline -funroll-loops -ftree-vectorize

all: main.x test.x

main.x:
	$(CC) $(CFLAGS) $(OPTFLAGS) -DNDEBUG main.c -o main.x

test.x:
	$(CC) $(CFLAGS) $(OPTFLAGS) test.c -o test.x

clean:
	rm -f main.x test.x
