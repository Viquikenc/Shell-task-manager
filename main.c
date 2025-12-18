/* if you are reading this, that means life has throwen us into this hell
 * together, it turns out C is just a curse, looks like we got a lot of haters,
 * anyway welcome my friend to a world where is no sleep and no happiness, hope
 * you enjoy my cursed code. Btw I don't accept judges about the code so keep it
 * for yourself. THANK YOU
 */

#include <dirent.h>
#include <ncurses.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Menu.h"
#include "error_handler.h"

#define BASE_10 10
#define NANO_PER_SEC 1000000000

FILE *err_file;
uint64_t total_mem;

static WINDOW *info_win;

static void *KeyHandler(void *);
static void SignalHandler(int signal);

static inline int initInfo(uint64_t *total_mem) {
  FILE *mem_file = fopen("/proc/meminfo", "r");
  if (mem_file) {
    if (fscanf(mem_file, "%*s %ld", total_mem)) {
      return SUCCESS;
    } else {
      ERR_SET(ERR_SCAN_FILE, FATAL);
      return ERR_SCAN_FILE;
    }
  } else {
    ERR_SET(ERR_OPEN_FILE, FATAL);
    return ERR_OPEN_FILE;
  }
}

int main(int argc, char *argv[]) {
  TableHeaderElementStruct TableList[MAX] = {
      {"PID", (int)strlen("PID"), PID_MARG, PID},
      {"Name", (int)strlen("Name"), NAME_MARG, NAME},
      {"User", (int)strlen("User"), USER_MARG, USER},
      {"PRI", (int)strlen("PRI"), PRI_MARG, PRI},
      {"NI", (int)strlen("NI"), NI_MARG, NI},
      {"VIRT", (int)strlen("VIRT"), VIRT_MARG, VIRT},
      {"RES", (int)strlen("REST"), RES_MARG, RES},
      {"SHR", (int)strlen("SHR"), SHR_MARG, SHR},
      {"S", (int)strlen("S"), S_MARG, S},
      {"CPU%", (int)strlen("CPU%"), CPU_MARG, CPU},
      {"MEM%", (int)strlen("MEM%"), MEM_MARG, MEM},
      {"Time", (int)strlen("Time"), TIME_MARG, TIME},
      {"Command", (int)strlen("Command"), 0, COMMAND},
  };

  err_file = fopen("errors.log", "a+");
  initInfo(&total_mem);
  initscr();
  cbreak();
  noecho();
  NewProccessElement Process;
  int xMax, yMax;
  getmaxyx(stdscr, yMax, xMax);
  info_win = newwin((3 * yMax) / 4, xMax, yMax / 4, 0);

  int current_pos = 1;
  int i = 0;

  do {
    mvprintw(yMax / 4 - 1, current_pos, "%s", TableList[i].name);
    current_pos += TableList[i].str_size + TableList[i].margin;
    i++;
  } while (i < MAX);

  mvchgat(yMax / 4 - 1, 0, xMax, A_STANDOUT, 0, NULL);
  refresh();
  wrefresh(info_win);
  keypad(info_win, TRUE);
  signal(SIGINT, SignalHandler);
  pthread_t pthread;
  pthread_create(&pthread, NULL, KeyHandler, NULL);
  struct timespec start, end;
  while (1) {
    refresh();
    clock_gettime(CLOCK_MONOTONIC, &start);
    uint16_t xpos = 1;
    uint16_t ypos = 0;
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
    clock_gettime(CLOCK_MONOTONIC, &end);
    long elapsed_ns = (end.tv_sec - start.tv_sec) * NANO_PER_SEC +
                      (end.tv_nsec - start.tv_nsec);
    if (elapsed_ns < NANO_PER_SEC) {
      long remaining_ns = NANO_PER_SEC - elapsed_ns;
      struct timespec sleep_time = {.tv_sec = 0, .tv_nsec = remaining_ns};
      nanosleep(&sleep_time, NULL);
      wclear(info_win);
    } else {
      wclear(info_win);
    }
  }
  endwin();
  return 0;
}

static void *KeyHandler(void *arg) {
  while (1) {
    int key = getch();
    if (key == 'q') {
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
