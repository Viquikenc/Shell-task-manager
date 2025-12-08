CC=gcc
CFLAGS=-lncurses -g
CWARN=-Wall -Werror -Wpedantic

main: main.c
	$(CC) main.c Menu.c error_handler.c -o main $(CFLAGS) $(CWARN)

clean:
	rm -f main
	rm -f errors.log
