#pragma once
#include<unistd.h>
#include<semaphore.h>

extern sem_t sem_log;
void logToFile(const char* message);
