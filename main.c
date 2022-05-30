#include <signal.h>
#include <stdlib.h>
#include "window.h"
#include "linked_list_proc.h"
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

static char* banner = "  _     _                     _                  \n | |   | |                   | |                 \n | |__ | |_ ___  _ __     ___| | ___  _ __   ___ \n | \'_ \\| __/ _ \\| \'_ \\   / __| |/ _ \\| \'_ \\ / _ \\\n | | | | || (_) | |_) | | (__| | (_) | | | |  __/\n |_| |_|\\__\\___/| .__/   \\___|_|\\___/|_| |_|\\___|\n                | |                              \n                |_|                              ";

void sigint_handler (int sig) {
	quit_flag = 1;
}

typedef struct {
	WINDOW* w_body;
	ListHead* list_head;
	int lower_bound;
	int upper_bound;
} PROCESSthread_arg_t;

void *thread_keyinput_func (void *arg) {
	PROCESSthread_arg_t* args = (PROCESSthread_arg_t*) arg;
	int nprocs = 0;
	while (!quit_flag) {
		sem_wait(&sem_readprocs);
		nprocs = getNumberOfProcesses(args->list_head);
		sem_post(&sem_readprocs);
		WINDOW* w_body = args->w_body;
		int ch = wgetch(w_body);
		keyinput_handler(w_body, ch, nprocs, &(args->lower_bound), &(args->upper_bound));
	}
	return NULL;
}



void *thread_processes_func(void *arg) {
	PROCESSthread_arg_t* arg_t = (PROCESSthread_arg_t*)arg;
	int* lower_bound = &(arg_t->lower_bound);
	int* upper_bound = &(arg_t->upper_bound);
	ListHead* list_head = arg_t->list_head;
	WINDOW* w_body = arg_t->w_body;
	int x, y;
	while (!quit_flag) {
		usleep(1000000);
		sem_wait(&sem_keyinput);
		getyx(w_body, y, x);
		sem_wait(&sem_readprocs);
		readProcs(list_head, w_body);
		sem_post(&sem_readprocs);
		werase(w_body);
		ListItemProcess* item = (ListItemProcess*)list_head->first;
		int i = 0;
		while (item != NULL) {
			if (i >= *lower_bound && i <= *upper_bound) wprintw(w_body, "  PID: %d    UID: %d    CPU USAGE: %f\n", item->process->pid, item->process->uid, item->process->cpu_usage);
            ListItem* aux = (ListItem*)item;
			item = (ListItemProcess*)aux->next;
			i++;
		}
		wmove(w_body, y, x);
		wrefresh(w_body);
		sem_post(&sem_keyinput);
		usleep(4000000);
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
	start_color();
	raw();
	noecho();
	WINDOW* w_header_wrapper = newwin(9, COLS, 0, 0);
	WINDOW* w_header = newwin(7, COLS-3, 1, 2);
	WINDOW* w_body_wrapper = newwin(LINES-12, COLS, 9, 0);
	WINDOW* w_body = newwin(LINES-14, COLS-2, 10, 1);
	WINDOW* w_footer_wrapper = newwin(3, COLS, LINES-3, 0);
	WINDOW* w_footer = newwin(1, COLS-2, LINES-2, 1);
	if (can_change_color() == TRUE)
	{
		init_color(COLOR_BLACK, 400, 400, 400);
		init_pair(1, COLOR_BLACK, COLOR_RED);
		wattron(w_header_wrapper, COLOR_PAIR(1));
		wattron(w_body_wrapper, COLOR_PAIR(1));
		wattron(w_footer_wrapper, COLOR_PAIR(1));
		wattron(w_header, COLOR_PAIR(1));
		wattron(w_body, COLOR_PAIR(1));
		wattron(w_footer, COLOR_PAIR(1));
	}
	int lower_bound = 0;
	int upper_bound = LINES-14;
	keypad(w_body, TRUE);
	scrollok(w_body, TRUE);
	idlok(w_body, TRUE);
	wsetscrreg(w_body, 1, LINES-14);
	box(w_header_wrapper, 0, 0);
	box(w_body_wrapper, 0, 0);
	box(w_footer_wrapper, 0, 0);
	wprintw(w_header, "%s", banner);
	wprintw(w_body, "Loading...\n");
	wprintw(w_footer, "(F1) - Terminate        (F2) - Kill        (F3) - Suspend       (F4) - Resume\n");
	wrefresh(w_header_wrapper);
	wrefresh(w_header);
	wrefresh(w_body_wrapper);
	wrefresh(w_body);
	wrefresh(w_footer_wrapper);
	wrefresh(w_footer);
	wmove(w_body, 0, 2);
	wrefresh(w_body);
	ListHead listProcesses;
	List_init(&listProcesses);
	pthread_t thread_keyinput;
	pthread_t thread_processes;
	sem_init(&sem_keyinput, 0, 1);
	sem_init(&sem_readprocs, 0, 1);
	PROCESSthread_arg_t arg_proc = {w_body, &listProcesses, lower_bound, upper_bound};
	PROCESSthread_arg_t arg_keyinput = {w_body, &listProcesses, lower_bound, upper_bound};
	pthread_create(&thread_keyinput, NULL, thread_keyinput_func, &arg_keyinput);
	pthread_create(&thread_processes, NULL, thread_processes_func, &arg_proc);
	while(1) {
		if(quit_flag) {
			break;
		}
		sem_wait(&sem_keyinput);
		wrefresh(w_body);
		sem_post(&sem_keyinput);
	}
	delwin(w_header);
	delwin(w_body);
	delwin(w_footer);
	sem_destroy(&sem_keyinput);
	pthread_join(thread_keyinput, NULL);
	pthread_join(thread_processes, NULL);
	ListItem* curr = (&listProcesses)->first;
	while (curr != NULL) {
		ListItem* aux = curr;
	    curr = curr->next;
		free(((ListItemProcess*)aux)->process);
	    free(aux);
	}
	endwin();
	return 0;
}

