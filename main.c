#include <dirent.h>
#include <ncurses.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Menu.h"
#include "error_handler.h"

#define BASE_10 10

FILE *err_file;

typedef enum TableHeaderElementsEnum {
  PID,
  USER,
  NAME,
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

static WINDOW *info_win;

static void *KillProgram(void *);
static void SignalHandler(int signal);

int main(int argc, char *argv[]) {
  err_file = fopen("errors.log", "a+");
  initscr();
  cbreak();
  noecho();
  NewProccessElement Process;
  int xMax, yMax;
  getmaxyx(stdscr, yMax, xMax);
  info_win = newwin((3 * yMax) / 4, xMax, yMax / 4, 0);

  TableHeaderElementStruct TableList[MAX] = {
      {"PID", (int)strlen("PID"), PID},
      {"User", (int)strlen("User"), USER},
      {"Name", (int)strlen("Name"), NAME},
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
    mvwprintw(info_win, 1, current_pos, "%s", TableList[i].name);
    current_pos += TableList[i].str_size + margin;
    i++;
  } while (i < MAX);

  mvwchgat(info_win, 1, 0, xMax, A_STANDOUT, 0, NULL);
  refresh();
  wrefresh(info_win);
  uint16_t ypos = 2;
  signal(SIGINT, SignalHandler);
  pthread_t pthread;
  pthread_create(&pthread, NULL, KillProgram, NULL);
  while (1) {
    uint16_t xpos = 1;
    pid_t pid = 1;
    DIR *dir = opendir("/proc");
    char *endptr;
    struct dirent *pid_dir;
    while ((pid_dir = readdir(dir))) {
      pid = strtol(pid_dir->d_name, &endptr, BASE_10);
      if (pid != 0) {
        if (GetProcessInfoFromFile(&Process, pid) == SUCCESS &&
            WinCreateProccessItem(info_win, xpos, ypos, Process) == SUCCESS) {
          ypos += 1;
          wrefresh(info_win);
        }
      }
    }
    closedir(dir);
    wrefresh(info_win);
  }
  endwin();
  return 0;
}

static void *KillProgram(void *arg) {
  while (1) {
    if (getch() == 'q') {
      delwin(info_win);
      endwin();
      exit(0);
    }
  }
  return 0;
}

static void SignalHandler(int signal) {
  delwin(info_win);
  endwin();
  puts("The program has been interepted");
  exit(1);
}
