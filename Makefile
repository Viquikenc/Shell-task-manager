CC=gcc

main: main.c
	$(CC) main.c Menu.c error_handler.c -o main -lncurses -g -Wall -Werror -Wpedantic

clean:
	rm -f main
	rm -f errors.log
