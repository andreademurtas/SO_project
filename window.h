#pragma once
#include <ncurses.h>
#include <semaphore.h>
#include "linked_list_proc.h"

extern int quit_flag;
extern sem_t sem_keyinput;
extern sem_t sem_readprocs;

void keyinput_handler(WINDOW *p_win, int ch, int nprocs, int* lower_bound, int* upper_bound, int* scrollHappened);
int checkbounds(int x, int y);
