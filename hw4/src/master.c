#include <stdlib.h>
#include <unistd.h>

#include "debug.h"
#include "polya.h"

struct worker {
    int pid;
    FILE* rd;
    FILE* wt;
    int state;
};

struct worker** workers_array;

void sigchild_handler();
int get_idle_worker(int workers);

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
            dup2(wt_pipe[0], 0);
            dup2(rd_pipe[1], 1);

            // exec
            char* argv[] = {NULL};
            if( execv("bin/polya_worker", argv)<0 ){
                perror("exec error\n");
                exit(EXIT_FAILURE);
            }
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
            child->state = WORKER_STARTED;
            workers_array[i] = child;
        }

    }

    // loop: assign problems to idle workers & post results
    while(1){
        int idle_worker_index = get_idle_worker(workers);
        struct worker* idle_worker = workers_array[idle_worker_index];

        // get problem
        struct problem* p = get_problem_variant(workers, idle_worker_index); // must not free this pointer

        // assign problem: send SIGCONT -> write problem
        kill(idle_worker->pid, SIGCONT);
        FILE* wt = idle_worker->wt;
        fwrite(p, p->size, 1, wt);
        fflush(wt);

        struct worker* stopped_worker = workers_array[0];
        // post result: read result -> set idle -> post result
        struct result* res = malloc(sizeof(struct result));
        FILE* rd = stopped_worker->rd;
        fread(res, sizeof(struct result), 1, rd); // get the header for the result
        res = realloc(res, res->size); // reallocate
        void* ptr = (char*)res + sizeof(struct result);
        fread(ptr, res->size - sizeof(struct result), 1, rd);

        stopped_worker->state = WORKER_IDLE; // set the state of the stopped child to idle

        if( post_result(res, p) == 0) { // is solution

        } else {

        }

    }

    // termination

    return EXIT_FAILURE;
}

void sigchild_handler() {
    // stop

    // continue
}

// catch or ignore SIGPIPE

int get_idle_worker(int workers) {
    for(int i=0; i<workers; i++) {
        struct worker* w = workers_array[i];
        if(w->state == WORKER_IDLE) {
            return i;
        }
    }
    return -1;
}