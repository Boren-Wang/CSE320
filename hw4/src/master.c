#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "debug.h"
#include "polya.h"

struct worker {
    pid_t pid;
    FILE* rd;
    FILE* wt;
    int state;
    struct problem* p;
};

int w; // number of workers
struct worker** workers_array;
volatile sig_atomic_t termsig;

void sigchild_handler();
struct worker *get_worker(pid_t pid);
int all_idle();

/*
 * master
 * (See polya.h for specification.)
 */
int master(int workers) {
    if(workers<=0){
        workers = 1;
    }
    w = workers;

    // add signal handlers: SIGCHILD
    if(signal(SIGCHLD, sigchild_handler)==SIG_ERR){
        perror("signal error");
    }

    if(signal(SIGPIPE, SIG_IGN)==SIG_ERR){ // ignore SIGPIPE???
        perror("signal error");
    }

    // block SIGCHLD

    // initialization: pipe->fork->redirection->exec
    struct worker** workers_array = malloc(workers*sizeof(struct worker)); // data structure to keep track of all file descriptors
    for(int i=0; i<workers; i++){
        pid_t pid;
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

    // unblock SIGCHLD

    // loop: assign problems to idle workers & post results
    int finished = 0;
    while(1){
        // little window
        pause(); // to be changed

        // read & post result: read result -> post result -> set child
        for(int i=0; i<w; i++) {
            struct worker* worker = workers_array[i];
            if(worker->state==WORKER_STOPPED){
                struct result* res = malloc(sizeof(struct result));
                FILE* rd = worker->rd;
                fread(res, sizeof(struct result), 1, rd); // get the header for the result
                res = realloc(res, res->size); // reallocate
                void* ptr = (char*)res + sizeof(struct result);
                fread(ptr, res->size - sizeof(struct result), 1, rd); // get the remaining for the result
                if( post_result(res, worker->p) == 0) { // is solution

                } else {

                }
                worker->state = WORKER_IDLE; // set the state of the stopped child to idle
                worker->p = NULL;
            }
        }

        // assign problem: send SIGCONT
        for(int i=0; i<w; i++) {
            struct worker* worker = workers_array[i];
            if(worker->state==WORKER_IDLE) {
                // get problem from problem source
                struct problem* p = get_problem_variant(workers, i); // must not free this pointer
                if(p==NULL) { // if there are no more problems
                    finished = 1;
                }
                worker->p = p;
                worker->state = WORKER_CONTINUED;
                kill(worker->pid, SIGCONT);
            }
        }

        // write problem to child(ren)
        for(int i=0; i<w; i++) {
            struct worker* worker = workers_array[i];
            if(worker->state==WORKER_RUNNING) {
                struct problem* p = worker->p;
                FILE* wt = worker->wt;
                fwrite(p, p->size, 1, wt);
                fflush(wt);
            }
        }

        if(finished==1){
            // check if all workers are idle
            if(all_idle()){

            }
        }
    }

    // termination

    return EXIT_FAILURE;
}

void sigchild_handler() {
    int olderrno = errno;
    pid_t pid;
    int status; // used to decide the state(s) of the child(ren)
    while( (pid=waitpid(-1, &status, WNOHANG))!=0 ){
        if(pid<0){ // error
            // ???
        }
        struct worker* worker = get_worker(pid);
        if(WIFSTOPPED(status)){ // if the child is stopped
            if(worker->state == WORKER_STARTED) {
                worker->state = WORKER_IDLE;
            } else if(worker->state == WORKER_RUNNING) {
                worker->state = WORKER_STOPPED;
            }
        } else if(WIFCONTINUED(status)) { // if the child is resumed by SIGCONT
            worker->state = WORKER_RUNNING;
        } else if(WIFEXITED(status)) { // if the child exits normally

        } else if(WIFSIGNALED(status)) { // if the child is terminated by signal
            termsig = WTERMSIG(status);
        }

    }

    errno = olderrno;
}

// catch or ignore SIGPIPE

struct worker *get_worker(pid_t pid) {
    for(int i=0; i<w; i++) {
        struct worker* worker = workers_array[i];
        if(worker->pid == pid) {
            return worker;
        }
    }
    return NULL;
}

int all_idle() {
    for(int i=0; i<w; i++) {
        struct worker* worker = workers_array[i];
        if(worker->state!=WORKER_IDLE){
            return 0;
        }
    }
    return 1;
}