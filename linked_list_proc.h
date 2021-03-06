#pragma once
#include <ncurses.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <ctype.h>
#include <pwd.h>
#include <unistd.h>
#include "utils.h"

struct ListItem;

typedef void (*ListItemPrintFn)(struct ListItem*);
typedef void (*ListItemDestroyFn)(struct ListItem*);

typedef struct {
    ListItemDestroyFn dtor_fn;
    ListItemPrintFn   print_fn;
} ListItemOps;

typedef struct ListItem {
    struct ListItem* prev;
    struct ListItem* next;
    ListItemOps* ops; // pointer to virtual method table
} ListItem;

typedef struct ListHead {
    ListItem* first;
    ListItem* last;
    int size;
} ListHead;

void List_init(ListHead* head);
ListItem* List_find(ListHead* head, ListItem* item);
ListItem* List_insert(ListHead* head, ListItem* previous, ListItem* item);
ListItem* List_detach(ListHead* head, ListItem* item);

typedef struct process {
	int pid;
	char* name;
	float cpu_usage;
	float mem_usage;
	unsigned int utime;
	unsigned int stime;
	unsigned int start_time;
	int still_running;
	int suspended;
} PROCESS;

typedef struct {
    ListItem list;
    PROCESS* process;
} ListItemProcess;

int checkIfPidExists(ListHead* head, int pid);
void readProcs(ListHead* head, int total_ram);
void calculateTotalCPUTime(float* uptime);
void calculateProcessTime(PROCESS* item);
ListItemProcess* findByPid(ListHead* head, int pid);
int getNumberOfProcesses(ListHead* head);
int calculateTotalRAM();
int calculateRAMProcess(PROCESS* item);
char* getName(PROCESS* item);
