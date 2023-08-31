#include <stdio.h>

#include "common_threads.h"

#define KEEP_ONE_ACCESS 1
#define ONLY_ONE_MUTEX 0
int balance = 0;
#if ONLY_ONE_MUTEX
static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
#endif

void* worker(void* arg) {
    balance++; // unprotected access 
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t p;
    Pthread_create(&p, NULL, worker, NULL);
    #if !KEEP_ONE_ACCESS
    #if ONLY_ONE_MUTEX
    pthread_mutex_lock(&lock);
    #endif
    balance++; // unprotected access
    #if ONLY_ONE_MUTEX
    pthread_mutex_unlock(&lock);
    #endif
    #endif
    Pthread_join(p, NULL);
    return 0;
}
