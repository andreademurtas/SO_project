#pragma once
#include <ncurses.h>

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

void ListItem_destroy(ListItem* item);
void ListItem_print(ListItem* item);

void ListItem_construct(ListItem* item, ListItemOps* ops);

typedef struct ListHead {
    ListItem* first;
    ListItem* last;
    int size;
} ListHead;

void List_init(ListHead* head);
ListItem* List_find(ListHead* head, ListItem* item);
ListItem* List_insert(ListHead* head, ListItem* previous, ListItem* item);
ListItem* List_detach(ListHead* head, ListItem* item);
void List_print(ListHead* head);

typedef struct process {
	int pid;
	int uid;
	int cpu_usage;
	int mem_usage;
	long long int utime_before;
	long long int stime_before;
	long long int utime_after;
	long long int stime_after;
	int still_running;
} PROCESS;

typedef struct {
    ListItem list;
    PROCESS* process;
} ListItemProcess;

int checkIfPidExists(ListHead* head, int pid);
void readProcs(ListHead* head,WINDOW* w_body);
void calculateTotalCPUTime(long long int* time_total_before, long long int* time_total_after);
void calculateProcessTime(PROCESS* item);
ListItemProcess* findByPid(ListHead* head, int pid);
