CC=gcc
CFLAGS=-lncurses -g
CWARN=-Wall -Werror -Wpedantic

main: main.c
	$(CC) main.c process.c error_handler.c Debug.c tui_menu.c -o main $(CFLAGS) $(CWARN)
noped:
	$(CC) main.c process.c error_handler.c Debug.c tui_menu.c -o main $(CFLAGS) -Wall -Werror
O3:
	$(CC) -O3 main.c process.c error_handler.c Debug.c tui_menu.c -o main $(CFLAGS)

clean:
	rm -f main
	rm -f errors.log
	rm -f debug.log
