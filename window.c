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
					//wscrl(p_win, -1);
					if (*lower_limit > 0) {(*lower_limit)--; sem_wait(&sem_log);logToFile("keyinput_handler: KEY_UP - scroll up");sem_post(&sem_log); wmove(p_win, LINES-14,x);}
					else {wmove(p_win, y, x);}
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
					//wscrl(p_win, 1);
					if (*lower_limit <= (nprocs - 1) / LINES-14){ (*lower_limit)++; sem_wait(&sem_log); logToFile("keyinput_handler: KEY_DOWN - scroll down");sem_post(&sem_log);wmove(p_win, 0, x);}
					else {wmove(p_win, y, x);}
				}
			}
			wrefresh(p_win);
			break;
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

