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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Debug.h"
#include "error_handler.h"
#include "process.h"
#include "tui_menu.h"

#define BASE_10 10

// global variables
FILE *err_file;
uint64_t total_mem;

static WINDOW *info_win = NULL;
static WINDOW *temp_win = NULL;

static Process *process = NULL;

static Menu info_menu;

static pthread_mutex_t mutex_lock;

static void *KeyHandler(void *);
static void SignalHandler(int);
static int initInfo(uint64_t *);

static int yMax;
static int xMax;

static int print_y = 0;

static int max_elements = 0;

static int cursor_x = 0;
// static int cursor_y = 0;

static int cam_y = 0;

static int PAD_X;
static int PAD_Y;

static TableHeaderElementStruct TableList[MAX] = {
    {"PID", (int)strlen("PID"), PID_MARG, PID},
    {"Name", (int)strlen("Name"), NAME_MARG, NAME},
    {"User", (int)strlen("User"), USER_MARG, USER},
    {"PRI", (int)strlen("PRI"), PRI_MARG, PRI},
    {"NI", (int)strlen("NI"), NI_MARG, NI},
    {"VIRT", (int)strlen("VIRT"), VIRT_MARG, VIRT},
    {"RES", (int)strlen("RES"), RES_MARG, RES},
    {"SHR", (int)strlen("SHR"), SHR_MARG, SHR},
    {"S", (int)strlen("S"), S_MARG, S},
    {"CPU%", (int)strlen("CPU%"), CPU_MARG, CPU},
    {"MEM%", (int)strlen("MEM%"), MEM_MARG, MEM},
    {"Time", (int)strlen("Time"), TIME_MARG, TIME},
    {"Command", (int)strlen("Command"), 0, COMMAND},
};

int main(void) {
  err_file = fopen("errors.log", "a");
  pid_t pid = 0;
  DIR *dir = NULL;
  struct dirent *pid_dir = NULL;
  initscr();
  cbreak();
  noecho();
  getmaxyx(stdscr, yMax, xMax);
  PAD_X = COLS - 1;
  PAD_Y = LINES - 1;
  /* this is a temp window for holding data in writing-time and it's made for
   * not interepting the the UI win and let the user see real-time responses
   */
  temp_win = newpad(PAD_Y + 100, PAD_X + 50);
  /* this is the displayed window which the data will be displayed at */
  info_win = newpad(PAD_Y + 100, PAD_X + 50);
  prefresh(info_win, cam_y, 0, yMax / 4, 0, PAD_Y, PAD_X);
  initInfo(&total_mem);

  if ((process = InitProcess()) == NULL) {
    delwin(info_win);
    delwin(temp_win);
    delwin(stdscr);
    FreeProcess(process);
    endwin();
    fclose(err_file);
    ERR_SET(ERR_PROCESS_INIT_FAIL, FATAL);
  }

  // enabling keys like (up-arrow, down-arrow, F1, F2, ...)
  keypad(info_win, TRUE);
  signal(SIGINT, SignalHandler);
  signal(SIGSEGV, SignalHandler);
  MenuSet(&info_menu, 3 * getmaxy(stdscr) / 4, max_elements, info_win);
  DebugWriteStringInfo("Maximum valid line in stdscr is");
  DebugWriteNumInfo(getmaxy(stdscr));
  DebugWriteNumInfo(getmaxx(stdscr));
  DebugWriteStringInfo("Maximium valid line in the sub-window is");
  DebugWriteNumInfo(getmaxy(info_win));
  DebugWriteNumInfo(getmaxx(info_win));
  pthread_mutex_init(&mutex_lock, NULL);
  pthread_t pthread;
  pthread_create(&pthread, NULL, KeyHandler, NULL);
  pthread_detach(pthread);
  // the start of the program.
  while (1) {
    print_y = 0;
    max_elements = 0;
    // erasing the content of the [info_win]
    werase(temp_win);
    dir = opendir("/proc");
    pid_dir = NULL;
    // reading every directory/file names in /proc
    while ((pid_dir = readdir(dir))) {
      pid = strtol(pid_dir->d_name, NULL, BASE_10);
      if (pid == 0)
        continue;
      if (GetProcessInfoFromFile(&process, pid) != SUCCESS)
        continue;
      PrintProcessItem(temp_win, *process, print_y++);
      ++max_elements;
    }
    closedir(dir);
    pthread_mutex_lock(&mutex_lock);
    /* overwriting(updating) the data stored in [temp_win] to the actual
     * displayed screen [info_win] */
    if (overwrite(temp_win, info_win) == ERR)
      ERR_SET(ERR_WIN_OVERWRITE_FAIL, FATAL);
    MenuUpdate(&info_menu, DCI, max_elements);
    // moving the cursor to a certain pos according to [cursor_y] &
    // [cursor_x]
    wmove(info_win, info_menu.item_selected_ypos, cursor_x);
    // refreshing the content of the pad
    prefresh(info_win, cam_y, 0, yMax / 4, 0, getmaxy(stdscr) - 1,
             getmaxx(stdscr) - 1);
    pthread_mutex_unlock(&mutex_lock);
    DebugWriteStringInfo("Maximum valid line in stdscr is");
    DebugWriteNumInfo(getmaxy(stdscr));
    DebugWriteNumInfo(getmaxx(stdscr));
    DebugWriteStringInfo("Maximium valid line in the sub-window is");
    DebugWriteNumInfo(getmaxy(info_win));
    DebugWriteNumInfo(getmaxx(info_win));
    sleep(3);
  }
  // freeing ncurses
  /* WARNING: there still many remaining allocated data that's not freed by
   * ncurses and I can't free it */
  endwin();
  return 0;
}

static void *KeyHandler(void *arg) {
  while (1) {
    int key = getch();
    if (key == 'q') {
      delwin(info_win);
      delwin(temp_win);
      delwin(stdscr);
      endwin();
      FreeProcess(process);
      pthread_mutex_destroy(&mutex_lock);
      fclose(err_file);
      exit(0);
    } else if (key == 'j') {
      pthread_mutex_lock(&mutex_lock);
      MenuSelectNext(&info_menu);
      pthread_mutex_unlock(&mutex_lock);
    } else if (key == 'k') {
      pthread_mutex_lock(&mutex_lock);
      MenuSelectPrev(&info_menu);
      pthread_mutex_unlock(&mutex_lock);
    }
  }
}

static int initInfo(uint64_t *total_mem) {
  FILE *mem_file = fopen("/proc/meminfo", "r");
  if (mem_file == NULL)
    ERR_SET(ERR_FILE_OPEN_FAIL, FATAL);
  if (fscanf(mem_file, "%*s %lu", total_mem) == EOF)
    ERR_SET(ERR_FILE_SCAN_FAIL, FATAL);
  for (int i = 0, current_pos = 1; i < MAX;
       current_pos += TableList[i].str_size + TableList[i].margin, ++i)
    mvprintw(yMax / 4 - 1, current_pos, "%s", TableList[i].name);

  mvchgat(yMax / 4 - 1, 0, xMax, A_STANDOUT, 0, NULL);
  refresh();
  fclose(mem_file);
  return SUCCESS;
}

static void SignalHandler(int signal) {
  delwin(info_win);
  delwin(stdscr);
  endwin();
  FreeProcess(process);
  pthread_mutex_destroy(&mutex_lock);
  fclose(err_file);
  puts("The program has been interepted");
  exit(1);
}
