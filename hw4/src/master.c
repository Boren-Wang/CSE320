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
int finished = 0;
int solved = 0;

void sigchild_handler();
struct worker *get_worker(pid_t pid);
void cancel_all();
int all_idle();
// void continue_all();
void terminate_all();
int all_terminated();
int some_aborted();

/*
 * master
 * (See polya.h for specification.)
 */
int master(int workers) {
    sf_start();
    if(workers<=0){
        workers = 1;
    }
    w = workers;
    sigset_t mask_all;
    sigset_t prev;
    sigset_t mask_sigsuspend;
    sigfillset(&mask_all);
    sigfillset(&mask_sigsuspend);
    sigdelset(&mask_sigsuspend, SIGINT);
    sigdelset(&mask_sigsuspend, SIGCHLD);

    // add signal handlers: SIGCHILD
    if(signal(SIGCHLD, sigchild_handler)==SIG_ERR){
        perror("signal error");
    }

    if(signal(SIGPIPE, SIG_IGN)==SIG_ERR){ // ignore SIGPIPE???
        perror("signal error");
    }

    // block SIGCHLD
    sigprocmask(SIG_BLOCK, &mask_all, &prev);

    // initialization: pipe->fork->redirection->exec
    workers_array = malloc(workers*sizeof(struct worker*)); // data structure to keep track of all file descriptors
    for(int i=0; i<workers; i++){
        // debug("initializing worker %d\n", i);
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
            // debug("performing redirection for worker %d\n", i);
            dup2(wt_pipe[0], 0);
            dup2(rd_pipe[1], 1);

            // exec
            // debug("execute worker %d", i);
            char* argv[] = {NULL};
            if( execv("bin/polya_worker", argv )<0 ){
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

            struct worker* child = malloc(sizeof(struct worker));
            child->pid = pid;
            child->rd = rd;
            child->wt = wt;
            child->state = WORKER_STARTED;
            sf_change_state(pid, 0, WORKER_STARTED);
            workers_array[i] = child;
        }

    }

    // unblock SIGCHLD
    sigprocmask(SIG_SETMASK, &prev, NULL);

    // debug("initialization is finished\n");

    // loop: assign problems to idle workers & post results
    while(1){
        // little window
        // debug("about to wait for a signal\n");
        sigsuspend(&mask_sigsuspend);
        // pause();
        // debug("a signal has arrived\n");

        // block signals
        sigprocmask(SIG_BLOCK, &mask_all, &prev);

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
                sf_recv_result(worker->pid, res);
                if( post_result(res, worker->p) == 0 ) { // is solution
                    cancel_all();
                    solved = 1;
                }
                worker->state = WORKER_IDLE; // set the state of the stopped child to idle
                sf_change_state(worker->pid, WORKER_STOPPED, WORKER_IDLE);
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
                    break;
                }
                worker->p = p;
                worker->state = WORKER_CONTINUED;
                sf_change_state(worker->pid, WORKER_IDLE, WORKER_CONTINUED);
                kill(worker->pid, SIGCONT);
            }
        }

        if(solved==0){ // if the current problem has not been solved
            // write problem to child(ren)
            for(int i=0; i<w; i++) {
                struct worker* worker = workers_array[i];
                if(worker->state==WORKER_RUNNING) {
                    struct problem* p = worker->p;
                    FILE* wt = worker->wt;
                    sf_send_problem(worker->pid, p);
                    fwrite(p, p->size, 1, wt);
                    fflush(wt);
                }
            }
        }

        if(finished==1){
            // check if all workers are idle
            if(all_idle()){
                // terminate_all(); // terminate all children
                break;
            }
        } else {
            solved = 0;
        }

        // unblock signals
        sigprocmask(SIG_SETMASK, &prev, NULL);
        // debug("conitune\n");
    }

    terminate_all(); // terminate all children
    // continue_all();
    debug("terminating\n");
    while(1) {
        // debug("about to wait for a signal\n");
        sigsuspend(&mask_sigsuspend);
        // pause();
        // debug("a signal has arrived");
        if(all_terminated()) { // check if all children are terminated
            debug("all children are terminated\n");
            sf_end();
            return EXIT_SUCCESS;
        } else if(some_aborted()) {
            debug("some children are aborted\n");
            sf_end();
            return EXIT_FAILURE;
        }
    }
    sf_end();
    return EXIT_FAILURE;
}

void sigchild_handler() { // to be changed
    // debug("sigchild_handler is called\n");
    int olderrno = errno;
    pid_t pid;
    int status; // used to decide the state(s) of the child(ren)
    int options = WNOHANG | WSTOPPED | WCONTINUED;
    while( (pid=waitpid(-1, &status, options))!=0 ){
    // while( (pid=wait(&status)) > 0 ) {
        debug("SIGCHLD from worker %d\n", pid);
        if(pid<0){ // error
            printf("wait error %d\n", errno);
        }
        struct worker* worker = get_worker(pid);
        int oldstate = worker->state;
        int newstate;
        if(WIFSTOPPED(status)){ // if the child is stopped
            debug("the child is stopped");
            if(worker->state == WORKER_STARTED) {
                worker->state = WORKER_IDLE;
            } else if(worker->state == WORKER_RUNNING) {
                worker->state = WORKER_STOPPED;
            }
            newstate = worker->state;
        }
        else if(WIFCONTINUED(status)) { // if the child is resumed by SIGCONT
            debug("the child is resumed by SIGCONT");
            worker->state = WORKER_RUNNING;
            newstate = worker->state;
        }
        else if(WIFEXITED(status)) { // if the child exits normally
            debug("the child exits normally");
            worker->state = WORKER_EXITED;
            newstate = worker->state;
        }
        else if(WIFSIGNALED(status)) { // if the child is terminated by signal
            debug("the child is terminated by signal");
            worker->state = WORKER_ABORTED;
            newstate = worker->state;
        } else {
            debug("else???");
        }
        sf_change_state(pid, oldstate, newstate);
    }
    errno = olderrno;
}

struct worker *get_worker(pid_t pid) {
    for(int i=0; i<w; i++) {
        struct worker* worker = workers_array[i];
        if(worker->pid == pid) {
            return worker;
        }
    }
    return NULL;
}

void cancel_all() {
    for(int i=0; i<w; i++) {
        struct worker* worker = workers_array[i];
        if(worker->state==WORKER_RUNNING){
            kill(worker->pid, SIGHUP);
        }
    }
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

// void continue_all() {
//     debug("continue_all is called\n");
//     for(int i=0; i<w; i++) {
//         struct worker* worker = workers_array[i];
//         kill(worker->pid, SIGCONT);
//     }
// }

void terminate_all() {
    debug("terminate_all is called\n");
    for(int i=0; i<w; i++) {
        struct worker* worker = workers_array[i];
        int ret = kill(worker->pid, SIGTERM);
        kill(worker->pid, SIGCONT);
        // debug("worker %d is terminated\n", worker->pid);
        printf("ret for kill is %d\n", ret);
    }
}

int all_terminated() {
    for(int i=0; i<w; i++) {
        struct worker* worker = workers_array[i];
        if(worker->state!=WORKER_EXITED) {
            return 0;
        }
    }
    return 1;
}

int some_aborted() {
    for(int i=0; i<w; i++) {
        struct worker* worker = workers_array[i];
        if(worker->state!=WORKER_EXITED || worker->state!=WORKER_ABORTED) {
            return 0;
        }
    }
    return 1;
}