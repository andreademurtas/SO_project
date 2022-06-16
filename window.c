#include "window.h"

int quit_flag = 0; 
sem_t sem_keyinput;
sem_t sem_readprocs;

void keyinput_handler (WINDOW *p_win, int ch, int nprocs, int* lower_limit){
	int x, y;
	sem_wait(&sem_keyinput);
    getyx(p_win, y, x);
	switch (ch) {
		case 'q':
			quit_flag = 1;
			break;
		case KEY_UP:
			if (y >= 0) {
				if (checkbounds(x,y-1)) {
					wmove(p_win, y-1, x);
				}
				else {
					wmove(p_win, y, x);
				}
			}
			wrefresh(p_win);
			break;
		case KEY_DOWN:
			if (y < nprocs - 1) {
				if (checkbounds(x,y+1)) {
					wmove(p_win, y+1, x);
				}
				else {
					wmove(p_win, y, x);
				}
			}
			wrefresh(p_win);
			break;
		case KEY_LEFT:
			if (*lower_limit > 0) {
				*lower_limit -= 1;
				wclear(p_win);
				wprintw(p_win, "  Loading...");
				wmove(p_win, 0, 2);
			}
			break;
		case KEY_RIGHT:
			if (*lower_limit < (nprocs + LINES - 13) / LINES - 14) {
				*lower_limit += 1;
				wclear(p_win);
				wprintw(p_win, "  Loading...");
				wmove(p_win, 0, 2);
				sem_wait(&sem_log);
				char* log =(char*)malloc(sizeof(char)*100);
				sprintf(log, "Lower limit: %d", *lower_limit);
				logToFile(log);
				free(log);
				sem_post(&sem_log);
			}
		default:
			break;
	}
	sem_post(&sem_keyinput);
}

int checkbounds(int x, int y) {
	if (x < 2 || x > COLS-2 || y < 0 || y > LINES-15) {
		return 0;
	}
	return 1;
}

