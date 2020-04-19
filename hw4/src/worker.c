#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

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
    // add signal handlers: SIGHUP & SIGTERM
    if(signal(SIGHUP, sighup_handler)==SIG_ERR){
        perror("signal error");
    }
    if(signal(SIGTERM, sigterm_handler)==SIG_ERR){
        perror("signal error");
    }

    // stop
    kill(getpid(), SIGSTOP);

    // loop
    while(1){
        // read problem
        debug("Worker reading result");
        struct problem* header = malloc(sizeof(struct problem));
        fread(header, sizeof(struct problem), 1, stdin); // read the header
        size_t size = header->size;
        struct problem* p = malloc(size);
        memcpy(p, header, sizeof(struct problem));
        free(header);
        void* ptr = (char*)p+sizeof(struct problem);
        fread(ptr, size-sizeof(struct problem), 1, stdin); // read the remaining

        // solve problem -> succeed/fail/cancel
        SOLVER* solver = solvers[p->type].solve;
        struct result* res = (*solver)(p, &canceled); // ???
        if(res == NULL){ // if the solver fails or is canceled
            res = malloc(sizeof(struct result));
            res->size = sizeof(struct result);
            res->id = p->id;
            res->failed = 1;
        }
        // write result
        debug("Worker writing result");
        fwrite((void*)res, res->size, 1, stdout);
        fflush(stdout);
        free(p);
        free(res);

        // stop
        debug("Worker being stopped");
        kill(getpid(), SIGSTOP);

        // reset conceled flag: is it safe??????????
        canceled = 0;
    }

    return EXIT_FAILURE;
}

void sighup_handler() {
    canceled = 1;
}

void sigterm_handler() {
    exit(EXIT_SUCCESS);
}
