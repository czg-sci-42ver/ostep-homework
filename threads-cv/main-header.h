#ifndef __main_header_h__
#define __main_header_h__

// all this is for the homework side of things

#include <time.h>
#include <errno.h>
#define CHECK_TIME

int do_trace = 0;
int do_timing = 0;

#define p0 do_pause(id, 1, 0, "p0"); 
#define p1 do_pause(id, 1, 1, "p1"); 
#define p2 do_pause(id, 1, 2, "p2"); 
#define p3 do_pause(id, 1, 3, "p3"); 
#define p4 do_pause(id, 1, 4, "p4"); 
#define p5 do_pause(id, 1, 5, "p5"); 
#define p6 do_pause(id, 1, 6, "p6"); 

#define c0 do_pause(id, 0, 0, "c0"); 
#define c1 do_pause(id, 0, 1, "c1"); 
#define c2 do_pause(id, 0, 2, "c2"); 
#define c3 do_pause(id, 0, 3, "c3"); 
#define c4 do_pause(id, 0, 4, "c4"); 
#define c5 do_pause(id, 0, 5, "c5"); 
#define c6 do_pause(id, 0, 6, "c6"); 

double producer_pause_times[MAX_THREADS][7];
double consumer_pause_times[MAX_THREADS][7];

// needed to avoid interleaving of print out from threads
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

void do_print_headers() {
    if (do_trace == 0) 
	return;
    int i;
    printf("%3s ", "NF");
    for (i = 0; i < max; i++) {
	printf(" %3s ", "   ");
    }
    printf("   ");

    for (i = 0; i < producers; i++) 
	printf("P%d ", i);
    for (i = 0; i < consumers; i++) 
	printf("C%d ", i);
    printf("\n");
}

void do_print_pointers(int index) {
    if (use_ptr == index && fill_ptr == index) {
	printf("*");
    } else if (use_ptr == index) {
	printf("u");
    } else if (fill_ptr == index) {
	printf("f");
    } else {
	printf(" ");
    }
}

void do_print_buffer() {
    int i;
    printf("%3d [", num_full);
    for (i = 0; i < max; i++) {
	do_print_pointers(i);
	if (buffer[i] == EMPTY) {
	    printf("%3s ", "---");
	} else if (buffer[i] == END_OF_STREAM) {
	    printf("%3s ", "EOS");
	} else {
	    printf("%3d ", buffer[i]);
	}
    }
    printf("] ");
}

void do_eos() {
    if (do_trace) {
	Mutex_lock(&print_lock);
	do_print_buffer();
	//printf("%3d [added end-of-stream marker]\n", num_full);
	printf("[main: added end-of-stream marker]\n");
	Mutex_unlock(&print_lock);
    }
}

void do_pause(int thread_id, int is_producer, int pause_slot, char *str) {
    int i;
    if (do_trace) {
	Mutex_lock(&print_lock);
	do_print_buffer();

	// skip over other thread's spots
	for (i = 0; i < thread_id; i++) {
	    printf("   ");
	}
	printf("%s\n", str);
	Mutex_unlock(&print_lock);
    }

    int local_id = thread_id;
    double pause_time;
    if (is_producer) {
	pause_time = producer_pause_times[local_id][pause_slot];
    } else {
	/*
	only one global `thread_id` in Pthread_create(&pid[i], NULL, producer, (void *) (long long) thread_id);
	So `thread_id - producers`
	*/
	local_id = thread_id - producers;
	pause_time = consumer_pause_times[local_id][pause_slot];
    }
    // printf(" PAUSE %f\n", pause_time);
	struct timespec ts;
	ts.tv_sec = (int)pause_time;
	ts.tv_nsec = (pause_time - ts.tv_sec)*1e+9;
	if (!((ts.tv_sec==0) && (ts.tv_nsec==0))) {
		#ifdef CHECK_TIME
		printf("%ld,%ld\n",ts.tv_sec,ts.tv_nsec); /*check_target*/
		fflush(stdout);
		#endif
		int res;
		struct timespec ts_rem;
		#ifdef CHECK_TIME
		printf("will begin nanosleep\n");
		#endif
		res = nanosleep(&ts, &ts_rem);
		#ifdef CHECK_TIME
		printf("end nanosleep 1st time\n");
		#endif
		while (res && errno == EINTR){
			#ifdef CHECK_TIME
			printf("got interrupted\n");
			#endif
			res = nanosleep(&ts, &ts_rem);
		}
		/*
		No "got interrupted"
		but "will begin nanosleep\n" is associated with "check_target"
		So nanosleep of different threads overlap.
		*/
		#ifdef CHECK_TIME
		printf("check whether sleep suspends all CPUs\n");
		#endif
		/*
		This may make stdout messy but it can show exactly when `nanosleep` finished.
		*/
		fflush(stdout);
		/*
		From https://www.geeksforgeeks.org/c-nanosleep-function/ and https://stackoverflow.com/a/1157217/21294350
		not use `ts_rem` to check.
		assert((ts.tv_sec==0) && (ts.tv_nsec==0));
		printf("remaining: %ld,%ld\n",ts_rem.tv_sec,ts_rem.tv_nsec);
		*/
	}
}

void ensure(int expression, char *msg) {
    if (expression == 0) {
	fprintf(stderr, "%s\n", msg);
	exit(1);
    }
}

void parse_pause_string(char *str, char *name, int expected_pieces, 
			double pause_array[MAX_THREADS][7]) {

    // string looks like this (or should):
    //   1,2,0:2,3,4,5
    //   n-1 colons if there are n producers/consumers
    //   comma-separated for sleep amounts per producer or consumer
    int index = 0;

    char *copy_entire = strdup(str);
    char *outer_marker;
    int colon_count = 0;
	/*
	strtok_r mainly to allow two interleaved different delimiters.
	*/
    char *p = strtok_r(copy_entire, ":", &outer_marker);
    while (p) {
	// init array: default sleep is 0
	int i;
	for (i = 0; i < 7; i++)  
	    pause_array[index][i] = 0;

	// for each piece, comma separated
	char *inner_marker;
	char *copy_piece = strdup(p);
	char *c = strtok_r(copy_piece, ",", &inner_marker);
	int comma_count = 0;

	int inner_index = 0;
	while (c) {
	    double pause_amount = atof(c);
	    ensure(inner_index < 7, "you specified a sleep string incorrectly... (too many comma-separated args)");
	    // printf("setting %s pause %d to %d\n", name, inner_index, pause_amount);
	    pause_array[index][inner_index] = pause_amount;
	    inner_index++;

	    c = strtok_r(NULL, ",", &inner_marker);	
	    comma_count++;
	}
	free(copy_piece);
	index++;

	// continue with colon separated list
	p = strtok_r(NULL, ":", &outer_marker);
	colon_count++;
	assert(index==colon_count);
    }

    free(copy_entire);
    if (expected_pieces != colon_count) {
	fprintf(stderr, "Error: expected %d %s in sleep specification, got %d\n", expected_pieces, name, colon_count);
	exit(1);
    }
}


#endif // __main_header_h__
