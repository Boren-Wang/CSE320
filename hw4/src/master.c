#include <stdlib.h>

#include "debug.h"
#include "polya.h"

/*
 * master
 * (See polya.h for specification.)
 */
int master(int workers) {
    // TO BE IMPLEMENTED
    if(workers<=0){
        workers = 1;
    }

    // initialization: pipe->fork->redirection->exec

    // add signal handlers: SIGCHILD

    // loop: assign problems to idle workers & post results

    // termination
    return EXIT_FAILURE;
}
