#include <signal.h>
#include <stdlib.h>
#include "window.h"
#include "linked_list_proc.h"
#include <pthread.h>
#include <unistd.h>

void sigint_handler (int sig) {
	quit_flag = 1;
}

void *thread_func (void *arg) {
	while (!quit_flag) {
		WINDOW* w_body = (WINDOW*)arg;
		int ch = wgetch(w_body);
		keyinput_handler(w_body, ch);
	}
	return NULL;
}

int main() {
	signal(SIGINT, sigint_handler);
	initscr();
	if(has_colors() == FALSE)
	{	
		endwin();
		printf("Your terminal does not support colors. Boring...\nQuitting.\n");
		exit(1);
	}
	//start_color();
	raw();
	noecho();
	WINDOW* w_header = newwin(9, COLS, 0, 0);
	WINDOW* w_body_wrapper = newwin(LINES-12, COLS, 9, 0);
	WINDOW* w_body = newwin(LINES-14, COLS-2, 10, 1);
	WINDOW* w_footer = newwin(3, COLS, LINES-3, 0);
	keypad(w_body, TRUE);
	wmove(w_body, 0, 0);
	scrollok(w_body, TRUE);
	idlok(w_body, TRUE);
	wsetscrreg(w_body, 0, LINES-14);
	box(w_header, 0, 0);
	box(w_body_wrapper, 0, 0);
	box(w_footer, 0, 0);
	wrefresh(w_header);
	wrefresh(w_body_wrapper);
	wrefresh(w_body);
	wrefresh(w_footer);
	wmove(w_body, 0, 2);
	wrefresh(w_body);
	ListHead listProcesses;
	List_init(&listProcesses);
	int i = 0;
	pthread_t thread;
	pthread_create(&thread, NULL, thread_func, w_body);
	while(1) {
		if(quit_flag) {
			break;
		}
		readProcs(&listProcesses);
		wrefresh(w_body);
		usleep(1000000);
	}
	delwin(w_header);
	delwin(w_body);
	delwin(w_footer);
	endwin();
	pthread_join(thread, NULL);
	return 0;
}

