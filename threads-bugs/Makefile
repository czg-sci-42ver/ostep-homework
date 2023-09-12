CFLAGS += -Wall -pthread -O
ALL = vector-deadlock vector-global-order vector-try-wait vector-avoid-hold-and-wait vector-nolock
COMMON = vector-header.h main-common.c main-header.h common.h common_threads.h

all: $(ALL)

clean:
	rm -f $(ALL) *~

vector-deadlock: vector-deadlock.c $(COMMON)
	gcc -o vector-deadlock vector-deadlock.c ${CFLAGS}

vector-global-order: vector-global-order.c $(COMMON)
	gcc -o vector-global-order vector-global-order.c ${CFLAGS}

vector-try-wait: vector-try-wait.c $(COMMON)
	gcc -o vector-try-wait vector-try-wait.c ${CFLAGS}

vector-avoid-hold-and-wait: vector-avoid-hold-and-wait.c $(COMMON)
	gcc -o vector-avoid-hold-and-wait vector-avoid-hold-and-wait.c ${CFLAGS}

vector-nolock: vector-nolock.c $(COMMON)
	gcc -o vector-nolock vector-nolock.c ${CFLAGS}

