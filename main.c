#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "window.h"
#include "linked_list_proc.h"
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "utils.h"
#include <stdint.h>
#include <errno.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


static char* banner = "  _____                  _    _      _                 \n |  __ \\                | |  | |    | |                \n | |__) | __ ___   ___  | |__| | ___| |_ __   ___ _ __ \n |  ___/ \'__/ _ \\ / __| |  __  |/ _ \\ | \'_ \\ / _ \\ \'__|\n | |   | | | (_) | (__  | |  | |  __/ | |_) |  __/ |   \n |_|   |_|  \\___/ \\___| |_|  |_|\\___|_| .__/ \\___|_|   \n                                      | |              \n                                      |_|";

static char* infos = "Version 1.0\n\nThis is a simple shell for interacting with process and displaying useful informations about them.\nType 'help' for a list of all the commands.\n";

static char* help = "\nhelp                Display this help\nquit                Exit the shell\nclear               Clear the screen\nps                  Display the list of processes\nkill [pid]          Kill a process\nterminate [pid]     Terminate a process \nsuspend [pid]       Suspend a process\nresume [pid]        Resume a process\ninteractive         Enter interactive mode\n";

/*
typedef struct {
	WINDOW* w_body;
	ListHead* list_head;
	int* lower_limit;
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
		keyinput_handler(w_body, ch, nprocs, args->lower_limit);
	}
	return NULL;
}
*/

void padString(char* str, int len_to_pad) {
	int len = strlen(str);
	if (len < len_to_pad) {
		int i;
		for (i = 0; i < len_to_pad - len; i++) {
			strcat(str, " ");
		}
	}
}

/*
void *thread_processes_func(void *arg) {
	PROCESSthread_arg_t* arg_t = (PROCESSthread_arg_t*)arg;
	int* lower_limit = arg_t->lower_limit;
	int total_ram = calculateTotalRAM();
	ListHead* list_head = arg_t->list_head;
	WINDOW* w_body = arg_t->w_body;
	int x, y;
	usleep(1000000);
	while (!quit_flag) {
		sem_wait(&sem_keyinput);
		getyx(w_body, y, x);
		readProcs(list_head, w_body, total_ram);
		werase(w_body);
		ListItemProcess* item = (ListItemProcess*)list_head->first;
		int i = 0;
		int old_lower_limit = *lower_limit;
		while (item != NULL) {
			char pid[6];
			sprintf(pid, "%d", item->process->pid);
			padString(pid, 5);
			sem_wait(&sem_log);
			char* log = (char*)malloc(sizeof(char)* 100);
			sprintf(log, "lower_limit: %d", *lower_limit);
			logToFile(log);
			free(log);
			sem_post(&sem_log);
			padString(item->process->name, 20);
			if (i >= (*lower_limit) * (LINES - 13) && i <= (*lower_limit) * (LINES - 14) + (LINES - 14)) {
				wprintw(w_body, "  PID: %s    NAME: %s    CPU USAGE: %%%.2f    MEM USAGE: %%%.2f    LINES: %d\n", pid, item->process->name, item->process->cpu_usage, item->process->mem_usage, LINES);
            	ListItem* aux = (ListItem*)item;
				item = (ListItemProcess*)aux->next;
				i++;
			} else if (old_lower_limit != *lower_limit) {
				old_lower_limit = *lower_limit;
				item = (ListItemProcess*)list_head->first;
				i = 0;
			}
			else {
				ListItem* aux = (ListItem*)item;
				item = (ListItemProcess*)aux->next;
				i++;
			}
		}
		wmove(w_body, y, x);
		wrefresh(w_body);
		sem_post(&sem_keyinput);
		uint16_t counter = 1;
		while (*lower_limit == old_lower_limit && counter != 0 && !quit_flag) {
            counter += 1;
			usleep(10);
		}
	}
	return NULL;
}
*/
void initNcurses() {
	initscr();
	if(has_colors() == FALSE)
	{	
		printf("Your terminal does not support colors. Boring...\nQuitting.\n");
		endwin();
		exit(1);
	}
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	scrollok(stdscr, TRUE);
	idlok(stdscr, TRUE);
	if (can_change_color()==TRUE){
		use_default_colors();
		init_pair(1, COLOR_BLACK, COLOR_WHITE);
		wbkgd(stdscr, COLOR_PAIR(1));
	}
}

/*
int main() {
	signal(SIGINT, sigint_handler);
	initNcurses();
	FIELD* fields[5];
	FORM* form;
	WINDOW* w_header_wrapper = newwin(9, COLS, 0, 0);
	WINDOW* w_header = newwin(7, COLS-3, 1, 2);
	WINDOW* w_body_wrapper = newwin(LINES-12, COLS, 9, 0);
	WINDOW* w_body = newwin(LINES-14, COLS-2, 10, 1);
	WINDOW* w_footer_wrapper = newwin(3, COLS, LINES-3, 0);
	WINDOW* w_footer = newwin(1, COLS-2, LINES-2, 1);
	if (can_change_color() == TRUE)
	{
		use_default_colors();
		init_pair(0xFF, COLOR_BLACK, COLOR_WHITE);
		wbkgd(w_header_wrapper, COLOR_PAIR(0xFF));
		wbkgd(w_header, COLOR_PAIR(0xFF));
		wbkgd(w_body_wrapper, COLOR_PAIR(0xFF));
		wbkgd(w_body, COLOR_PAIR(0xFF));
		wbkgd(w_footer_wrapper, COLOR_PAIR(0xFF));
		wbkgd(w_footer, COLOR_PAIR(0xFF));
	}
	wrefresh(w_header_wrapper);
	wrefresh(w_header);
	wrefresh(w_body_wrapper);
	wrefresh(w_body);
	wrefresh(w_footer_wrapper);
	wrefresh(w_footer);
	lower_limit = (int*)malloc(sizeof(int));
	*lower_limit = 0;
	keypad(w_body, TRUE);
	scrollok(w_body, TRUE);
	idlok(w_body, TRUE);
	wsetscrreg(w_body, 1, LINES-14);
	box(w_header_wrapper, 0, 0);
	box(w_body_wrapper, 0, 0);
	box(w_footer_wrapper, 0, 0);
	wprintw(w_header, "%s", banner);
	wprintw(w_body, "  Loading...\n");
	wprintw(w_footer, "(F1) - Terminate    (F2) - Kill    (F3) - Suspend   (F4) - Resume    (->) - Go To Next Page    (<-) - Go To Previous Page    (q) - Quit\n");
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
	sem_init(&sem_log, 0, 1);
	PROCESSthread_arg_t arg_proc = {w_body, &listProcesses, lower_limit};
	PROCESSthread_arg_t arg_keyinput = {w_body, &listProcesses, lower_limit};
	pthread_create(&thread_keyinput, NULL, thread_keyinput_func, &arg_keyinput);
	pthread_create(&thread_processes, NULL, thread_processes_func, &arg_proc);
	pthread_join(thread_keyinput, NULL);
	pthread_join(thread_processes, NULL);
	sem_destroy(&sem_keyinput);
	sem_destroy(&sem_readprocs);
	sem_destroy(&sem_log);
	delwin(w_header);
	delwin(w_body);
	delwin(w_footer);
	delwin(w_header_wrapper);
	delwin(w_body_wrapper);
	delwin(w_footer_wrapper);
	ListItem* curr = (&listProcesses)->first;
	while (curr != NULL) {
		ListItem* aux = curr;
	    curr = curr->next;
		free(((ListItemProcess*)aux)->process->name);
		free(((ListItemProcess*)aux)->process);
	    free(aux);
	}
	free(lower_limit);
	endwin();
	return 0;
}*/

void ps(ListHead* head, int total_ram) {
    readProcs(head, total_ram);
	ListItemProcess* proc = (ListItemProcess*)(head->first);
	uint8_t i = 0;
	while (proc) {
		char pid[6];
		char number[9];
		sprintf(number, "%d)", i);
		sprintf(pid, "%d", proc->process->pid);
		pid[5] = '\0';
		number[8] = '\0';
		padString(pid, 5);	
		padString(number, 5);
		padString(proc->process->name, 20);
		char susp_or_run[10];
		if (proc->process->suspended) {
			strcpy(susp_or_run, "Suspended");
		} else {
			strcpy(susp_or_run, "Running");
		}
		printf("%s PID: %s    NAME: %s    CPU USAGE: %%%.2f    MEM USAGE: %%%.2f    %s\n", number, pid, proc->process->name, proc->process->cpu_usage, proc->process->mem_usage, susp_or_run);
		ListItem* aux = (ListItem*)proc;
		proc = (ListItemProcess*)aux->next;
		i++;
	}
}

void interactive(int ncursesInitialized) {
    if (!ncursesInitialized) {
		initNcurses();
	}
	refresh();
	usleep(1000000);
	endwin();
}

void arg_handler(char* argv[]){
    exit(0);
}

int main(int argc, char* argv[]) {
	if (argc > 1) {
		arg_handler(argv);
	}
	int total_ram = calculateTotalRAM();
	ListHead listProcesses;
	List_init(&listProcesses);
    printf(ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "\n", banner);
	printf("%s\n", infos);
	char* command = (char*)malloc(sizeof(char)*100);
	int ncursesInitialized = 0;
	while (1) {
        printf(ANSI_COLOR_RED "prochelper> " ANSI_COLOR_RESET);
		memset(command, 0, 100);
		char* ret = fgets(command, 100, stdin);
		if (ret == NULL) {
			break;
		}
		command[strlen(command)-1] = '\0';
		char* token = strtok(command, " ");
		if (strcmp(token, "quit") == 0 || strcmp(token, "q") == 0) {
			break;
		}
		if (strcmp(token, "help") == 0) {
			printf(ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET "\n", help);
		}
		if (strcmp(token, "clear") == 0) {
			system("clear");
		}
		if (strcmp(token, "ps") == 0) {
			ps(&listProcesses, total_ram);
		}
		if (strcmp(token, "interactive") == 0) {
			interactive(ncursesInitialized);
			if (ncursesInitialized == 0) {
				ncursesInitialized = 1;
			}
		}
		if (strcmp(token, "terminate") == 0) {
            token = strtok(NULL, " ");
			if (token == NULL) {
                printf("You must specify a PID.\n");
			}
			else {
			    int check = 0;
				for (const char* c = token; *c != '\0'; c++) {
					if (!isdigit(*c)) {
						check = 1;
						break;
					}
				}
				if (check) {
                    printf("Invalid PID.\n");
				}
				else {
					int pid = atoi(token);
					int ret = kill(pid, SIGTERM);
					if (ret == 0) {
						printf("Process %d terminated.\n", pid);
					}
					else {
						printf("Process %d not found.\n", pid);
					}
				}
			}
		}
		if (strcmp(token, "kill") == 0) {
			token = strtok(NULL, " ");
			if (token == NULL) {
				printf("You must specify a PID.\n");
			}
			else {
			    int check = 0;
				for (const char* c = token; *c != '\0'; c++) {
					if (!isdigit(*c)) {
						check = 1;
						break;
					}
				}
				if (check) {
					printf("Invalid PID.\n");
				}
				else {
					int pid = atoi(token);
					int ret = kill(pid, SIGKILL);
					if (ret == 0) {
						printf("Process %d killed.\n", pid);
					}
					else {
						printf("Process %d not found.\n", pid);
					}
				}
			}
		}
		if (strcmp(token, "suspend") == 0) {
		    token = strtok(NULL, " ");
			if (token == NULL) {
				printf("You must specify a PID.\n");
			}
			else {
			    int check = 0;
				for (const char* c = token; *c != '\0'; c++) {
					if (!isdigit(*c)) {
						check = 1;
						break;
					}
				}
				if (check) {
					printf("Invalid PID.\n");
				}
				else {
					int pid = atoi(token);
					ListItemProcess* proc = findByPid(&listProcesses, pid);
					if (proc == NULL) {
                        printf("Process %d not found.\n", pid);
					}
					if (proc->process->suspended) {
						printf("Process %d already suspended.\n", pid);
					}
					else {
						int ret = kill(pid, SIGSTOP);
						if (ret == 0) {
							printf("Process %d suspended.\n", pid);
							proc->process->suspended = 1;
						}
						else {
							printf("Process %d not found.\n", pid);
						}
				    }
				}
			}
		}
		if (strcmp(token, "resume") == 0) {
		    token = strtok(NULL, " ");
			if (token == NULL) {
                printf("You must specify a PID.\n");
			}
			else {
			    int check = 0;
				for (const char* c = token; *c != '\0'; c++) {
					if (!isdigit(*c)) {
						check = 1;
						break;
					}
				}
				if (check) {
					printf("Invalid PID.\n");
				}
				else {
					int pid = atoi(token);
					ListItemProcess* proc = findByPid(&listProcesses, pid);
					if (proc == NULL) {
                        printf("Process %d not found.\n", pid);
					}
					if (!proc->process->suspended) {
					    printf("Process is not suspended.\n");
					}
					else {
						int ret = kill(pid, SIGCONT);
						if (ret == 0) {
							printf("Process %d resumed.\n", pid);
							proc->process->suspended = 0;
						}
						else {
							printf("Process %d not found.\n", pid);
						}
					}
				}
			}
		}
	}

	ListItem* curr = (&listProcesses)->first;
	while (curr != NULL) {
		ListItem* aux = curr;
	    curr = curr->next;
		free(((ListItemProcess*)aux)->process->name);
		free(((ListItemProcess*)aux)->process);
	    free(aux);
	}

	free(command);
}
