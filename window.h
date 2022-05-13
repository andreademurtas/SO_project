#pragma once
#include <ncurses.h>
#include <semaphore.h>

extern int quit_flag;
extern sem_t sem_keyinput;

void keyinput_handler(WINDOW *p_win, int ch);
int checkbounds(int x, int y);
