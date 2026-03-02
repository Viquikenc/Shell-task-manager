#include <ncurses.h>
#include <stdint.h>

#include "tui_menu.h"

void MenuSet(Menu *menu, const uint16_t page_max_, const uint16_t item_num_max_,
             WINDOW *win) {
  menu->item_selected_ypos = 0;
  menu->page_min = 0;
  menu->page_max = page_max_;
  menu->item_num_max = item_num_max_;
  menu->win = win;
}

void MenuUpdate(Menu *menu, const uint16_t page_max_,
                const uint16_t item_num_max_) {
  menu->page_max = (page_max_ == DCI) ? menu->page_max : page_max_;
  menu->item_num_max =
      (item_num_max_ == DCI) ? menu->item_num_max : item_num_max_;
}

void MenuSelectNext(Menu *menu) {
  if ((menu->item_selected_ypos + 1) > menu->item_num_max)
    return;
  ++menu->item_selected_ypos;
  menu->page_max += (menu->item_selected_ypos > menu->page_max) ? 1 : 0;
  menu->page_min += (menu->item_selected_ypos > menu->page_max) ? 1 : 0;
  wmove(menu->win, menu->item_selected_ypos, 0);
  prefresh(menu->win, getmaxy(stdscr) / 4, 0, menu->page_min, 0,
           getmaxy(stdscr) - 1, getmaxx(stdscr) - 1);
  return;
}

void MenuSelectPrev(Menu *menu) {
  if (menu->item_selected_ypos == 0 ||
      menu->item_selected_ypos - 1 > menu->page_min)
    return;
  --menu->item_selected_ypos;
  menu->page_min -= (menu->item_selected_ypos > menu->page_min) ? 1 : 0;
  menu->page_max -= (menu->item_selected_ypos > menu->page_min) ? 1 : 0;
  wmove(menu->win, menu->item_selected_ypos, 0);
  prefresh(menu->win, menu->page_min, 0, getmaxy(menu->win) / 4, 0,
           getmaxy(stdscr) - 1, getmaxx(stdscr) - 1);
  return;
}
