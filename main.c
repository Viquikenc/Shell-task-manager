#include <dirent.h>
#include <ncurses.h>
#include <pthread.h>
#include <signal.h>
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
            WinCreateProccessItem(Info_win, xpos, ypos, Process) == SUCCESS) {
          ypos += 1;
          refresh();
          wrefresh(Info_win);
        }
      }
    }
    closedir(dir);
    refresh();
    wrefresh(Info_win);
    usleep(2000 * 1000);
  }
  endwin();
  return 0;
}

static void *KillProgram(void *arg) {
  while (1) {
    if (getch() == 'q') {
      endwin();
      exit(0);
    }
  }
  return 0;
}

static void SignalHandler(int signal) {
  endwin();
  puts("The program has been interepted");
  exit(1);
}
