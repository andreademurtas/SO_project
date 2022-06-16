#pragma once
#include <ncurses.h>
#include <form.h>
#include <semaphore.h>
#include "linked_list_proc.h"
#include <stdlib.h>
#include "utils.h"

extern int quit_flag;
extern sem_t sem_keyinput;
extern sem_t sem_readprocs;

void keyinput_handler(WINDOW *p_win, int ch, int nprocs, int* lower_limit);
int checkbounds(int x, int y);
