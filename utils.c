#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

sem_t sem_log;

void logToFile(const char* message) {
	FILE* fp = fopen("log.txt", "a");
	if (fp == NULL) {
		printf("LOG - Error opening file!\n");
		exit(1);
	}
	fprintf(fp, "%s\n", message);
	fclose(fp);
}
