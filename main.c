#include <curses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <menu.h>
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

void padString(char* str, int len_to_pad) {
	int len = strlen(str);
	if (len < len_to_pad) {
		int i;
		for (i = 0; i < len_to_pad - len; i++) {
			strcat(str, " ");
		}
	}
}
void initNcurses() {
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	scrollok(stdscr, TRUE);
	idlok(stdscr, TRUE);
}

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

void menu(ListHead* head, int total_ram) {
    ITEM** proc_items;
	ITEM* curr_item;
	MENU* proc_menu;
	int c;
    readProcs(head, total_ram);
	int n_proc_items = getNumberOfProcesses(head);
	char** processes_choices = (char**)malloc(sizeof(char*) * (n_proc_items + 1));
	for (int i = 0; i < n_proc_items; i++) {
		processes_choices[i] = (char*)malloc(sizeof(char) * 500);
	}
	proc_items = (ITEM**)malloc(sizeof(ITEM*) * n_proc_items);
	ListItemProcess* proc = (ListItemProcess*)(head->first);
	short i = 0;
    while(proc) {
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
		sprintf(processes_choices[i], "%s PID: %s    NAME: %s    CPU USAGE: %%%.2f    MEM USAGE: %%%.2f    %s", number, pid, proc->process->name, proc->process->cpu_usage, proc->process->mem_usage, susp_or_run);
		proc_items[i] = new_item(processes_choices[i], "");
		proc = (ListItemProcess*)(((ListItem*)proc)->next);
		i++;		
	}
	proc_items[n_proc_items] = (ITEM*)NULL;
	proc_menu = new_menu((ITEM**)proc_items);
	mvprintw(LINES - 2, 0, "q - Quit    Enter - Select    Arrow keys - Move    R - Refresh");
	post_menu(proc_menu);
	refresh();

	while ((c = getch()) != 'q') {
		switch (c) {
			case KEY_DOWN:
				menu_driver(proc_menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				menu_driver(proc_menu, REQ_UP_ITEM);
				break;
			case 'r':
				readProcs(head, total_ram);
				int old_n_proc_items = n_proc_items;
				for (int j = 0; j < old_n_proc_items; j++) {
					free(processes_choices[j]);
					free_item(proc_items[j]);
				}
				free(processes_choices);
				free(proc_items);
				n_proc_items = getNumberOfProcesses(head);
				proc_items = (ITEM**)malloc(sizeof(ITEM*) * (n_proc_items + 1));
				processes_choices = (char**)malloc(sizeof(char*) * n_proc_items);
				for (int i = 0; i < n_proc_items; i++) {
					processes_choices[i] = (char*)malloc(sizeof(char) * 500);
				}
				proc = (ListItemProcess*)(head->first);
				i = 0;
				while(proc) {
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
					sprintf(processes_choices[i], "%s PID: %s    NAME: %s    CPU USAGE: %%%.2f    MEM USAGE: %%%.2f    %s", number, pid, proc->process->name, proc->process->cpu_usage, proc->process->mem_usage, susp_or_run);
					proc_items[i] = new_item(processes_choices[i], "");
					proc = (ListItemProcess*)(((ListItem*)proc)->next);
					i++;
			    }
				proc_items[n_proc_items] = (ITEM*)NULL;
				unpost_menu(proc_menu);
				proc_menu = new_menu((ITEM**)proc_items);
				post_menu(proc_menu);
				mvprintw(LINES - 2, 0, "q - Quit    Enter - Select    Arrow keys - Move    R - Refresh");
				refresh();
				break;
			case '\n':
				curr_item = current_item(proc_menu);
				const char* curr_item_str = item_name(curr_item);
				char pid[6];
				size_t j = 0;
				size_t i = 0;
				while (curr_item_str[i] != 'P') {
					i++;
				}
				i += 5;
				while (curr_item_str[i] != ' ') {
					pid[j] = curr_item_str[i];
					i++;
					j++;
				}
				pid[j] = '\0';
				int pid_int = atoi(pid);
			    ITEM** options = (ITEM**)malloc(sizeof(ITEM*) * 5);
				options[0] = new_item("1) Suspend", "");
				options[1] = new_item("2) Resume", "");
				options[2] = new_item("3) Kill", "");
				options[3] = new_item("4) Terminate", "");
				options[4] = (ITEM*)NULL;
				MENU* menu_options = new_menu((ITEM**)options);
				WINDOW* menu_win = newwin(LINES - 2, COLS, 0, 0);
				keypad(menu_win, TRUE);
				set_menu_win(menu_options, menu_win);
				set_menu_sub(menu_options, derwin(menu_win, LINES - 2, COLS, 0, 0));
				set_menu_mark(menu_options, " * ");
				mvprintw(LINES - 2, 0, "q - Quit    Enter - Select    Arrow keys - Move    R - Refresh");
				refresh();
				post_menu(menu_options);
				wrefresh(menu_win);
				int C;
				int check = 0;
				while ((C = getch()) != 'q')  {
					switch (C) {
						case KEY_DOWN:
							menu_driver(menu_options, REQ_DOWN_ITEM);
							break;
						case KEY_UP:
							menu_driver(menu_options, REQ_UP_ITEM);
							break;
						case '\n':
							curr_item = current_item(menu_options);
							const char* curr_item_str = item_name(curr_item);
							if (strcmp(curr_item_str, "1) Suspend") == 0) {
								ListItemProcess* proc = findByPid(head, pid_int);
								if (proc == NULL) {
									continue;
								}
							    if (proc->process->suspended) {
									continue;
								}
								else {
									int ret = kill(pid_int, SIGSTOP);
									if (ret == 0) {
										proc->process->suspended = 1;
										check = 1;
									}
									else {
										continue;
									}
								}
							} else if (strcmp(curr_item_str, "2) Resume") == 0) {
								ListItemProcess* proc = findByPid(head, pid_int);
								if (proc == NULL) {
									continue;
								}
							    if (!proc->process->suspended) {
									continue;	
								}
								else {
									int ret = kill(pid_int, SIGCONT);
									if (ret == 0) {
										proc->process->suspended = 0;
										check = 1;
									}
									else {
										continue;
									}
								}
							} else if (strcmp(curr_item_str, "3) Kill") == 0) {
								ListItemProcess* proc = findByPid(head, pid_int);
								if (proc == NULL) {
									continue;
								}
								else {
									int ret = kill(pid_int, SIGKILL);
									if (ret == 0) {
										check = 1;
									}
									else {
										continue;
									}
								}
							} else if (strcmp(curr_item_str, "4) Terminate") == 0) {
								ListItemProcess* proc = findByPid(head, pid_int);
								if (proc == NULL) {
									continue;
								}
								else {
									int ret = kill(pid_int, SIGTERM);
									if (ret == 0) {
										check = 1;
									}
									else {
										continue;
									}
								}

							}

					}
					wrefresh(menu_win);
					if (check) break;
	    		}
				for (int k = 0; k<5; k++) free_item(options[k]);
				free(options);
				unpost_menu(menu_options);
				free_menu(menu_options);
				wrefresh(menu_win);
				delwin(menu_win);
				touchwin(stdscr);
				menu_driver(proc_menu, REQ_FIRST_ITEM);
				break;
		}
	}

    for (int i = 0; i < n_proc_items; i++) {
		free(processes_choices[i]);
		free_item(proc_items[i]);
	}
	
	free(processes_choices);
	free(proc_items);
	unpost_menu(proc_menu);
	free_menu(proc_menu);
	endwin();
}

void interactive(int ncursesInitialized, ListHead* head, int total_ram) {
    if (!ncursesInitialized) {
		initNcurses();
	}
	refresh();
	menu(head, total_ram);
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
		if (command[0] == '\0') {
			continue;
		}
		if (strcmp(token, "quit") == 0 || strcmp(token, "q") == 0) {
			break;
		}
		if (strcmp(token, "help") == 0) {
			printf(ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET "\n", help);
		}
		if (strcmp(token, "clear") == 0) {
			(void)!system("clear");
		}
		if (strcmp(token, "ps") == 0) {
			ps(&listProcesses, total_ram);
		}
		if (strcmp(token, "interactive") == 0) {
			interactive(ncursesInitialized, &listProcesses, total_ram);
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
