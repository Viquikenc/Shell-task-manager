CC=gcc

main: main.c
	$(CC) main.c Menu.c -o main -lncurses -g -Wall -Werror -Wpedantic

clean:
	rm -f main
