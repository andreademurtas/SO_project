#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <ctype.h>

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
		printf("Total CPU time: %d\n", totalCPU);
}

void calculateProcessTime(int pid){
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
		printf("Process CPU utime: %d\n", utime);
        token = strtok(NULL, " ");
        int stime = atoi(token);
		printf("Process CPU stime: %d\n", stime);
}

void readProcs(){
	int total_time_before, total_time_after;
	int pid;
    char *path_proc[] = { "/proc", NULL };
    FTS* fts_proc = fts_open(path_proc, FTS_PHYSICAL | FTS_NOSTAT, NULL);
    FTSENT* node;
    if (NULL != fts_proc) {
                while (NULL != (node = fts_read(fts_proc))) {
					    int check = 0;
                        for (const char* p = node->fts_name; *p; ++p) {
                                if (!isdigit(*p)) {
										check = 1;
										break;
                                }
                        }
						if (check == 0){
							printf("PID: %s\n", node->fts_name);
                        	pid = atoi(node->fts_name);
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
                        	if (pid > 0) {
						     	calculateTotalCPUTime();
							 	calculateProcessTime(pid);
                        	}
						}
						if (strcmp(node->fts_name, "proc")){
                            fts_set(fts_proc, node, FTS_SKIP);
						}
						printf("\n");
                }
        fts_close(fts_proc);
        }
        else {
                perror("fts_open");
                exit(EXIT_FAILURE);
        }
}


int main() {
	readProcs();
	return 0;
}
