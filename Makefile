CFLAGS=-Wall -Wextra -pedantic -ggdb -std=c11
CLIBS=-lraylib

main: main.c
	$(CC) $(CFLAGS) main.c $(CLIBS) -o main
