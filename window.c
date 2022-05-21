#include "window.h"

int quit_flag = 0; 
sem_t sem_keyinput;
sem_t sem_readprocs;

void keyinput_handler (WINDOW *p_win, int ch, int nprocs) {
	int x, y;;
    getyx(p_win, y, x);
	switch (ch) {
		case 'q':
			quit_flag = 1;
			break;
		case KEY_UP:
			if (y > 1) {
				if (checkbounds(x,y-1)) {
					wmove(p_win, y-1, x);
				}
				else {
					wscrl(p_win, -1);
					wmove(p_win, y, x);
				}
			}
			sem_wait(&sem_keyinput);
			wrefresh(p_win);
			sem_post(&sem_keyinput);
			break;
		case KEY_DOWN:
			if (y < nprocs) {
				if (checkbounds(x,y+1)) {
					wmove(p_win, y+1, x);
				}
				else {
					wscrl(p_win, 1);
					wmove(p_win, y, x);
				}
			}
			sem_wait(&sem_keyinput);
			wrefresh(p_win);
			sem_post(&sem_keyinput);
			break;
		default:
			break;
	}
}

int checkbounds(int x, int y) {
	if (x < 2 || x > COLS-2 || y < 1 || y > LINES-15) {
		return 0;
	}
	return 1;
}
