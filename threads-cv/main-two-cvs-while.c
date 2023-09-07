#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>

#include "common.h"
#include "common_threads.h"

#include "pc-header.h"

// used in producer/consumer signaling protocol
pthread_cond_t empty  = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill   = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m     = PTHREAD_MUTEX_INITIALIZER;

#include "main-header.h"

/*
This cause output like
"""
$ ./main-two-cvs-while -l 3 -m 5 -p 1 -c 1 -v
 NF                             P0 C0 
  0 [*---  ---  ---  ---  --- ] p0
  0 [*---  ---  ---  ---  --- ]    c0
  0 [*---  ---  ---  ---  --- ] p1
  1 [u  0 f---  ---  ---  --- ] p4
...
"""
here fill_ptr and use_ptr are inited with 0.
then `p4` after `do_fill(base + i);` will filled with buffer[0] with 0 and `fill_ptr` becomes 1.
*/
void do_fill(int value) {
    // ensure empty before usage
    ensure(buffer[fill_ptr] == EMPTY, "error: tried to fill a non-empty buffer");
    buffer[fill_ptr] = value;
    fill_ptr = (fill_ptr + 1) % max;
    num_full++;
}

int do_get() {
    int tmp = buffer[use_ptr];
    ensure(tmp != EMPTY, "error: tried to get an empty buffer");
    buffer[use_ptr] = EMPTY; 
    use_ptr = (use_ptr + 1) % max;
    num_full--;
    return tmp;
}

/*
Here different producers produce different data
while different consumers don't care the data consumed.
So no the ostep book "covering condition" problem.
*/
void *producer(void *arg) {
    int id = (int) arg;
    // make sure each producer produces unique values
    int base = id * loops; 
    int i;
    for (i = 0; i < loops; i++) {   p0;
	Mutex_lock(&m);             p1;
	while (num_full == max) {   p2;
	    Cond_wait(&empty, &m);  p3;
	}
	do_fill(base + i);          p4;
	Cond_signal(&fill);         p5;
	Mutex_unlock(&m);           p6;
    }
    return NULL;
}
                                                                               
void *consumer(void *arg) {
    int id = (int) arg;
    int tmp = 0;
    int consumed_count = 0;
    int wait_full = 0;
    while (tmp != END_OF_STREAM) { c0;
	Mutex_lock(&m);            c1;
    wait_full = 0;
	while (num_full == 0) {    c2;
	    Cond_wait(&fill, &m);  c3;
        wait_full++;
        }
    if (wait_full>1) {
        printf("consumer has waited too long (i.e. %d times)\n",wait_full);
    }
	tmp = do_get();            c4;
	Cond_signal(&empty);       c5;
	Mutex_unlock(&m);          c6;
	consumed_count++;
    }

    // return consumer_count-1 because END_OF_STREAM does not count
    return (void *) (long long) (consumed_count - 1);
}

// must set these appropriately to use "main-common.c"
pthread_cond_t *fill_cv = &fill;
pthread_cond_t *empty_cv = &empty;

// all codes use this common base to start producers/consumers
// and all the other related stuff
#include "main-common.c"



