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

int time_total_after=0, time_total_before=0;

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
    // we check that the previous is inthe list
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

void readProcs(ListHead* head){
	char *path_proc[] = { "/proc", NULL };
    FTS* fts_proc = fts_open(path_proc, FTS_PHYSICAL | FTS_NOSTAT, NULL);
	FTSENT* node;
    if (NULL != fts_proc) {
		while (NULL != (node = fts_read(fts_proc))) {
			if (node->fts_info & FTS_F) {
				for (int i = 0; i < node->fts_namelen; i++) {
					if (!isdigit(node->fts_name[i])) {
						continue;
					}
				}
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
				if (pid > 0 && !checkIfPidExists(head, pid)) {
					ListItemProcess* item = (ListItemProcess*)malloc(sizeof(ListItemProcess));
					item->process = (PROCESS*)malloc(sizeof(PROCESS));
					item->process->pid = pid;
					calculateProcessTime(item->process);
					item->process->uid = uid;
					item->process->cpu_usage = 100 * (item->process->utime_after - item->process->utime_before) / (time_total_after - time_total_before);
					item->process->mem_usage = 0;
					List_insert(head, 0, (ListItem*)item);
				}
			}
			fts_set(fts_proc, node, FTS_SKIP);
		}
	fts_close(fts_proc);
	}
	else {
		perror("fts_open");
		exit(EXIT_FAILURE);
	}
}

void calculateTotalCPUTime(){
	FILE* fCPUStat = fopen("/proc/stat", "r");
	if (fCPUStat == NULL) {
		perror("fopen");
		exit(1);
	}
	char line[1024];
	fgets(line, sizeof(line), fCPUStat);
	fclose(fCPUStat);
	char* token = strtok(line, " ");
	token = strtok(NULL, " ");
	int totalCPU = 0;
	while (token != NULL) {
		totalCPU += atoi(token);
		token = strtok(NULL, " ");
	}
	time_total_before = time_total_after;
	time_total_after = totalCPU;
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
	int utime = atoi(token);
	token = strtok(NULL, " ");
	int stime = atoi(token);
	item->utime_before = item->utime_after;
	item->utime_after = utime;
	item->stime_before = item->stime_after;
	item->stime_after = stime;
}
