#include "linked_list_proc.h"
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

void ListItem_construct(ListItem* item, ListItemOps* ops) {
    item->prev=item->next=0;
    item->ops=ops;
}

void ListItem_destroy(ListItem* item) {
    assert(item);
    if (item->ops && item->ops->dtor_fn)
      (*item->ops->dtor_fn)(item);
}

void ListItem_print(ListItem* item) {
    assert(item);
    assert(item->ops && item->ops->print_fn);
    (*item->ops->print_fn)(item);
}

void List_init(ListHead* head) {
    head->first=0;
    head->last=0;
    head->size=0;
}

ListItem* List_find(ListHead* head, ListItem* item) {
    // linear scanning of list
    ListItem* aux=head->first;
    while(aux){
      if (aux==item)
        return item;
      aux=aux->next;
    }
    return 0;
}

ListItem* List_insert(ListHead* head, ListItem* prev, ListItem* item) {
    if (item->next || item->prev)
      return 0;
  
#ifdef _LIST_DEBUG_
    // we check that the element is not in the list
    ListItem* instance=List_find(head, item);
    assert(!instance);

    // we check that the previous is inthe list

    if (prev) {
      ListItem* prev_instance=List_find(head, prev);
      assert(prev_instance);
    }
    // we check that the previous is in the list
#endif

    ListItem* next= prev ? prev->next : head->first;
    if (prev) {
      item->prev=prev;
      prev->next=item;
    }
    if (next) {
      item->next=next;
      next->prev=item;
    }
    if (!prev)
      head->first=item;
    if(!next)
      head->last=item;
    ++head->size;
    return item;
  }

ListItem* List_detach(ListHead* head, ListItem* item) {

#ifdef _LIST_DEBUG_
    // we check that the element is in the list
    ListItem* instance=List_find(head, item);
    assert(instance);
#endif

    ListItem* prev=item->prev;
    ListItem* next=item->next;
    if (prev){
      prev->next=next;
    }
    if(next){
      next->prev=prev;
    }
    if (item==head->first)
      head->first=next;
    if (item==head->last)
      head->last=prev;
    head->size--;
    item->next=item->prev=0;
    return item;
}

void List_print(ListHead* head) {
    ListItem* item=head->first;
    printf("Printing polimorphic list\n");
    int k=0;
    while(item) {
      printf("l[%d]: ",k);
      ListItem_print(item);
      item=item->next;
      ++k;
    }
}

int checkIfPidExists(ListHead* head, int pid) {
    ListItem* item=head->first;
    while(item) {
	    if (((ListItemProcess*)item)->process->pid==pid)
	      return 1;
	    item=item->next;
	}
    return 0;
}

void readProcs(ListHead* head, WINDOW* w_body){
	unsigned int total_time = 0;
    calculateTotalCPUTime(&total_time);
	char *path_proc[] = { "/proc", NULL };
    FTS* fts_proc = fts_open(path_proc, FTS_PHYSICAL | FTS_NOSTAT, NULL);
	FTSENT* node;
	ListItemProcess* item = (ListItemProcess*)head->first;
	while (item) {
		item->process->still_running = 0;
		item = (ListItemProcess*)(((ListItem*)item)->next);
	}
    if (NULL != fts_proc) {
		int i = 0;
		while (NULL != (node = fts_read(fts_proc))) {
			int check = 0;
			for (const char* p =node->fts_name; *p; p++) {
				if (!isdigit(*p)) {
					check = 1;
					break;
				}
			}
			if (!check) {
				int pid = atoi(node->fts_name);
				int uid = 0;
				char pathUid[100];
				snprintf(pathUid, 100, "/proc/%d/status", pid);
				FILE* uidFile = fopen(pathUid, "r");
				if (uidFile) {
					fscanf(uidFile, "%d", &uid);
					fclose(uidFile);
				}
				else {
					continue;
				}
				if (i==1) {i=0; continue;}
				if (i==0) i=1;
				int exists = checkIfPidExists(head, pid);
				if (pid > 0 && !exists) {
					ListItemProcess* item = (ListItemProcess*)malloc(sizeof(ListItemProcess));
					ListItem* aux = (ListItem*)item;
                    aux->prev = aux->next = 0;
					item->process = (PROCESS*)malloc(sizeof(PROCESS));
					item->process->pid = pid;
					calculateProcessTime(item->process);
					item->process->uid = uid;
					unsigned int heartz = sysconf(_SC_CLK_TCK);
					unsigned int seconds = total_time  - item->process->start_time;
					unsigned int tot_proc = (item->process->utime + item->process->stime) / heartz;
					unsigned int cpu_usage = 100 * (tot_proc / seconds);
					item->process->cpu_usage = cpu_usage;
					item->process->mem_usage = 0;
					item->process->still_running = 1;
					List_insert(head, NULL, (ListItem*)item);
				}
				else if (exists) {
					ListItemProcess* item = findByPid(head, pid);
					calculateProcessTime(item->process);
					unsigned int heartz = sysconf(_SC_CLK_TCK);
					unsigned int seconds = total_time  - item->process->start_time;
					unsigned int tot_proc = (item->process->utime + item->process->stime) / heartz;
					unsigned int cpu_usage = 100 * (tot_proc / seconds);
					item->process->cpu_usage = cpu_usage;
					item->process->still_running = 1;
				}
			}
			if (strcmp(node->fts_name, "proc")){
				fts_set(fts_proc, node, FTS_SKIP);
			}
		}
	fts_close(fts_proc);
	}
	else {
		perror("fts_open");
		exit(EXIT_FAILURE);
	}
	item = (ListItemProcess*)head->first;
	while (item) {
		ListItemProcess* next = (ListItemProcess*)(((ListItem*)item)->next);
		if (!item->process->still_running) {
			List_detach(head, (ListItem*)item);
			free(item->process);
			free(item);
		}
		item = next;
	}
}

void calculateTotalCPUTime(unsigned int* total_time) {
	FILE* fCPUStat = fopen("/proc/uptime", "r");
	if (fCPUStat == NULL) {
		perror("fopen");
		exit(1);
	}
	char line[1024];
	fgets(line, sizeof(line), fCPUStat);
	fclose(fCPUStat);
	char* token = strtok(line, " ");
	*total_time = atof(token);
}

void calculateProcessTime(PROCESS* item){
    int pid = item->pid;
	char path[100];
	snprintf(path, 100, "/proc/%d/stat", pid);
	FILE* fProcessStat = fopen(path, "r");
	if (fProcessStat == NULL) {
		perror("fopen");
		exit(1);
	}
	char line[1024];
	fgets(line, sizeof(line), fProcessStat);
	fclose(fProcessStat);
	int i = 0;
	char* token = strtok(line, " ");
    while (i < 13) {
		token = strtok(NULL, " ");
		i++;
	}
	unsigned int utime = atoi(token);
	token = strtok(NULL, " ");
	i++;
	unsigned int stime = atoi(token);
	while (i < 21) {
		token = strtok(NULL, " ");
		i++;
	}
    unsigned int start_time = atoi(token);
    unsigned int heartz = sysconf(_SC_CLK_TCK);
	item->utime = utime;
	item->stime = stime;
	item->start_time = start_time / heartz;
}

ListItemProcess* findByPid(ListHead* head, int pid) {
    ListItem* item = head->first;
	while(item) {
		if (((ListItemProcess*)item)->process->pid==pid)
			return (ListItemProcess*)item;
		item=item->next;
	}
	return NULL;
}

int getNumberOfProcesses(ListHead* head) {
	ListItem* item = head->first;
	int count = 0;
	while (item) {
		count++;
		item = item->next;
	}
	return count;
}
