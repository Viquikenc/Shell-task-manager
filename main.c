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
#define MAX_LINES 2000
#define MAX_COLUMN 1700
FILE *err_file;
uint64_t total_mem;

static WINDOW *info_win;

static void *KeyHandler(void *);
static void SignalHandler(int);
static inline int initInfo(uint64_t *);

static int yMax;
static int cam_y = 0;
static int cursor_y = 0;

static int PAD_X;
static int PAD_Y;

int main(void) {
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
  PAD_X = COLS - 1;
  PAD_Y = LINES - 1;
  NewProccessElement Process;
  int xMax;
  getmaxyx(stdscr, yMax, xMax);
  info_win = newpad(PAD_Y + 100, PAD_X + 50);
  int current_pos = 1;
  int i = 0;
  do {
    mvprintw(yMax / 4 - 1, current_pos, "%s", TableList[i].name);
    current_pos += TableList[i].str_size + TableList[i].margin;
    i++;
  } while (i < MAX);
  mvchgat(yMax / 4 - 1, 0, xMax, A_STANDOUT, 0, NULL);
  refresh();
  prefresh(info_win, cam_y, 0, yMax / 4, 0, PAD_Y, PAD_X);
  keypad(info_win, TRUE);
  signal(SIGINT, SignalHandler);
  pthread_t pthread;
  pthread_create(&pthread, NULL, KeyHandler, NULL);
  pthread_detach(pthread);
  struct timespec start, end;
  while (1) {
    werase(info_win);
    prefresh(info_win, cam_y, 0, yMax / 4, 0, PAD_Y, PAD_X);
    clock_gettime(CLOCK_MONOTONIC, &start);
    pid_t pid = 1;
    DIR *dir = opendir("/proc");
    char *endptr;
    struct dirent *pid_dir;
    while ((pid_dir = readdir(dir))) {
      pid = strtol(pid_dir->d_name, &endptr, BASE_10);
      if (pid != 0) {
        if (GetProcessInfoFromFile(&Process, pid) == SUCCESS &&
            WinCreateProccessItem(info_win, Process) == SUCCESS) {
          prefresh(info_win, cam_y, 0, yMax / 4, 0, PAD_Y, PAD_X);
        }
      }
    }
    closedir(dir);
    clock_gettime(CLOCK_MONOTONIC, &end);
    long elapsed_ns = (end.tv_sec - start.tv_sec) * NANO_PER_SEC +
                      (end.tv_nsec - start.tv_nsec);
    if (elapsed_ns < NANO_PER_SEC) {
      long remaining_ns = NANO_PER_SEC - elapsed_ns;
      struct timespec sleep_time = {.tv_sec = 2, .tv_nsec = remaining_ns};
      nanosleep(&sleep_time, NULL);
    }
    prefresh(info_win, cam_y, 0, yMax / 4, 0, PAD_Y, PAD_X);
  }
  endwin();
  return 0;
}

static void *KeyHandler(void *arg) {
  int max_y = getmaxy(stdscr) - (yMax / 4);
  cursor_y = 0;
  int min_y = 0;
  while (1) {
    int key = getch();
    if (key == 'q') {
      delwin(info_win);
      delwin(stdscr);
      endwin();
      exit(0);
    } else if (key == 'j') {
      if ((++cursor_y) >= max_y) {
        ++min_y, ++max_y;
        wmove(info_win, cursor_y, 0);
        prefresh(info_win, ++cam_y, 0, yMax / 4, 0, PAD_Y, PAD_X);
      } else {
        wmove(info_win, cursor_y, 0);
        prefresh(info_win, cam_y, 0, yMax / 4, 0, PAD_Y, PAD_X);
      }
    } else if (key == 'k') {
      if ((--cursor_y) < min_y) {
        min_y -= (min_y <= 0) ? 0 : 1;
        max_y -= (max_y < (getmaxy(stdscr) - (yMax / 4))) ? 0 : 1;
        cursor_y = (cursor_y < 0) ? 0 : cursor_y;
        wmove(info_win, cursor_y, 0);
        prefresh(info_win, --cam_y, 0, yMax / 4, 0, PAD_Y, PAD_X);
      } else {
        wmove(info_win, cursor_y, 0);
        prefresh(info_win, cam_y, 0, yMax / 4, 0, PAD_Y, PAD_X);
      }
    }
  }
}

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

static void SignalHandler(int signal) {
  delwin(info_win);
  endwin();
  puts("The program has been interepted");
  exit(1);
}
