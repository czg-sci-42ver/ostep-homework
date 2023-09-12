#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "common_threads.h"

#include "main-header.h"
#include "vector-header.h"

// taken from https://en.wikipedia.org/wiki/Fetch-and-add
/*
AT&T size suffix https://stackoverflow.com/questions/20247944
movq https://stackoverflow.com/questions/27990177 is already in x86 https://www.felixcloutier.com/x86/movd:movq
here att syntax put the dst

With "-O" default inline.
*/
int fetch_and_add(int * variable, int value) {
    asm volatile("lock; xaddl %%eax, %1"
		 :"+a" (value), "+m" (*variable)
		 :
		 :"memory");
    return value;
}

void vector_add(vector_t *v_dst, vector_t *v_src) {
    int i;
    for (i = 0; i < VECTOR_SIZE; i++) {
	fetch_and_add(&v_dst->values[i], v_src->values[i]);
    }
}

void fini() {}


#include "main-common.c"

