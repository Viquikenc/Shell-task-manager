#ifndef __TUI_MENU_H__
#define __TUI_MENU_H__

#include <ncurses.h>
#include <stdint.h>

#define DCI 0 // Don't Change It

typedef struct menu_ Menu;

struct menu_ {
  uint16_t item_selected_ypos;
  uint16_t page_min;
  uint16_t page_max;
  uint16_t item_num_max;
  WINDOW *win;
};

void MenuSet(Menu *, const uint16_t page_max, const uint16_t item_num_max,
             WINDOW *);
void MenuUpdate(Menu *, const uint16_t page_max, const uint16_t item_num_max);
void MenuSelectNext(Menu *);
void MenuSelectPrev(Menu *);

#endif
