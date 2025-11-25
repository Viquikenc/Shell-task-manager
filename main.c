#include <bits/time.h>
#include <dirent.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Menu.h"
#include "error_handler.h"

#define BASE_10 10

typedef enum TableHeaderElementsEnum {
  PID,
  USER,
  PRI,
  NI,
  VIRT,
  RES,
  SHR,
  S,
  CPU,
  MEM,
  TIME,
  COMMAND,
  MAX
} TableHeaderElementsEnum;

typedef struct TableHeaderElementStruct {
  char *name;
  int str_size;
  TableHeaderElementsEnum pos;
} TableHeaderElementStruct;

int main(int argc, char *argv[]) {
  initscr();
  cbreak();
  noecho();
  NewProccessElement Process;
  int xMax, yMax;
  getmaxyx(stdscr, yMax, xMax);
  WINDOW *Info_win = newwin((3 * yMax) / 4, xMax, yMax / 4, 0);

  TableHeaderElementStruct TableList[MAX] = {
      {"PID", (int)strlen("PID"), PID},
      {"User", (int)strlen("User"), USER},
      {"PRI", (int)strlen("PRI"), PRI},
      {"NI", (int)strlen("NI"), NI},
      {"VIRT", (int)strlen("VIRT"), VIRT},
      {"RES", (int)strlen("REST"), RES},
      {"SHR", (int)strlen("SHR"), SHR},
      {"S", (int)strlen("S"), S},
      {"CPU", (int)strlen("CPU"), CPU},
      {"MEM", (int)strlen("MEM"), MEM},
      {"Time", (int)strlen("Time"), TIME},
      {"Command", (int)strlen("Command"), COMMAND},
  };
  int current_pos = 1;
  int i = 0;
  short margin = 2;

  do {
    mvwprintw(Info_win, 1, current_pos, "%s", TableList[i].name);
    current_pos += TableList[i].str_size + margin;
    i++;
  } while (i <= COMMAND);

  mvwchgat(Info_win, 1, 0, xMax, A_STANDOUT, 0, NULL);
  refresh();
  wrefresh(Info_win);
  while (1) {
    uint16_t xpos = 1;
    uint16_t ypos = 2;
    pid_t pid = 1;
    DIR *dir = opendir("/proc");
    char *endptr;
    struct dirent *pid_dir;
    while ((pid_dir = readdir(dir))) {
      pid = strtol(pid_dir->d_name, &endptr, BASE_10);
      if (pid != 0) {
        GetProcessInfoFromFile(&Process, pid);
        WinCreateProccessItem(Info_win, xpos, ypos, Process);
        wrefresh(Info_win);
        refresh();
        ++ypos;
      }
    }
    refresh();
    sleep(8);
  }
  endwin();
  return 0;
}
