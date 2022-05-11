#include "window.h"

int quit_flag = 0; 

void keyinput_handler (WINDOW *p_win, int ch) {
	int x, y;;
    getyx(p_win, y, x);
	switch (ch) {
		case 'q':
			quit_flag = 1;
			break;
		case KEY_UP:
			if (checkbounds(x,y-1)) wmove(p_win, y-1, x);
			break;
		case KEY_DOWN:
			if (checkbounds(x,y+1)) wmove(p_win, y+1, x);
			break;
		case KEY_LEFT:
			if (checkbounds(x-1,y)) wmove(p_win, y, x-1);
			break;
		case KEY_RIGHT:
			if (checkbounds(x+1,y)) wmove(p_win, y, x+1);
			break;
		default:
			break;
	}
}

int checkbounds(int x, int y) {
	if (x < 2 || x > COLS-2 || y < 0 || y > LINES-14) {
		return 0;
	}
	return 1;
}
