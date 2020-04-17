#include <stdlib.h>

#include "debug.h"
#include "polya.h"

void sighup_handler();
void sigterm_handler();

volatile sig_atomic_t canceled=0;

/*
 * worker
 * (See polya.h for specification.)
 */
int worker(void) {
    // TO BE IMPLEMENTED

    // add signal handlers: SIGHUP & SIGTERM
    if(signal(SIGHUP, sighup_handler)==SIG_ERR){
        perror("signal error");
    }
    if(signal(SIGTERM, sighup_handler)==SIG_ERR){
        perror("signal error");
    }

    // SIGCON

    // loop
    while(1){
        if(canceled==0){ // ?????
            // read problem
            struct problem* p = (struct problem*)(malloc(sizeof(struct problem)));
            fread(p, sizeof(struct problem), 1, stdin); // read the header
            size_t size = p->size;
            fread(p->data, size-sizeof(struct problem), 1, stdin); // read the remaining

            // solve problem -> succeed/fail/cancel
            SOLVER* solver = solvers[p->type].solve;
            struct result* res = solver(p, &canceled);

            // write result
            fwrite(stdout, res->size, 1, (void*)res);

            // stop
        }
    }

    return EXIT_FAILURE;
}

void sighup_handler() {
    canceled = 1;
}

void sigterm_handler() {
    exit(EXIT_SUCCESS);
}
