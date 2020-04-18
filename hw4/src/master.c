#include <stdlib.h>
#include <unistd.h>

#include "debug.h"
#include "polya.h"

struct worker {
    int pid;
    FILE* rd;
    FILE* wt;
    int status;
};

void sigchild_handler();

/*
 * master
 * (See polya.h for specification.)
 */
int master(int workers) {
    if(workers<=0){
        workers = 1;
    }

    // add signal handlers: SIGCHILD
    if(signal(SIGCHLD, sigchild_handler)==SIG_ERR){
        perror("signal error");
    }

    // initialization: pipe->fork->redirection->exec
    struct worker** workers_array = malloc(workers*sizeof(struct worker)); // data structure to keep track of all file descriptors
    for(int i=0; i<workers; i++){
        int pid;
        FILE *rd, *wt;

        int rd_pipe[2]; // pipe for reading from child
        if( pipe(rd_pipe)<0 ) {
            perror("Can't create pipe");
            exit(EXIT_FAILURE);
        }

        int wt_pipe[2]; // pipe for writing to child
        if( pipe(wt_pipe)<0 ) {
            perror("Can't create pipe");
            exit(EXIT_FAILURE);
        }

        if( (pid=fork()) == 0 ) { // child
            close(rd_pipe[0]); // close read side of the read pipe
            close(wt_pipe[1]); // close write side of the write pipe

            // redirection

            // exec
        } else { // parent
            close(rd_pipe[1]); // close write side of the read pipe
            close(wt_pipe[0]); // close read side of the write pipe

            if( (rd = fdopen(rd_pipe[0], "r"))==NULL ){
                perror("Master can't create input stream");
                exit(1);
            }

            if( (wt = fdopen(wt_pipe[1], "w"))==NULL ){
                perror("Master can't create output stream");
                exit(1);
            }

            struct worker* child = malloc(sizeof(child));
            child->pid = pid;
            child->rd = rd;
            child->wt = wt;
            child->status = WORKER_STARTED;
            workers_array[i] = child;
        }

    }

    // loop: assign problems to idle workers & post results
    while(1){
        struct problem* p = get_problem_variant(1, 1);
    }

    // termination

    return EXIT_FAILURE;
}

void sigchild_handler() {
    // stop

    // continue
}

// catch or ignore SIGPIPE
