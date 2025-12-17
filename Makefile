CC=gcc
CFLAGS=-lncurses -g
CWARN=-Wall -Werror -Wpedantic

main: main.c
	$(CC) main.c Menu.c error_handler.c Debug.c -o main $(CFLAGS) $(CWARN)
nopedantic:
	$(CC) main.c Menu.c error_handler.c Debug.c -o main $(CFLAGS) -Wall -Werror

clean:
	rm -f main
	rm -f errors.log
	rm -f debug.log
