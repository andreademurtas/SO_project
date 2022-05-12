#pragma once
#include <ncurses.h>

extern int quit_flag;

void keyinput_handler(WINDOW *p_win, int ch);
int checkbounds(int x, int y);
