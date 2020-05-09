#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "pbx.h"
#include "server.h"
#include "debug.h"
#include "csapp.h"

static void terminate(int status);
static void sighup_handler();

extern char *optarg;

/*
 * "PBX" telephone exchange simulation.
 *
 * Usage: pbx <port>
 */
int main(int argc, char* argv[]){
    int listenfd, *connfdp;
    char* port;
    socklen_t clientlen = sizeof(struct sockaddr_in);;
    struct sockaddr_in clientaddr;
    pthread_t tid;

    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    int option = getopt(argc, argv, "p:");
    if( option == 'p' ){
        port = optarg;
    }

    // Perform required initialization of the PBX module.
    debug("Initializing PBX...");
    pbx = pbx_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function pbx_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.
    struct sigaction action, old_action;
    action.sa_handler = sighup_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;

    if( sigaction(SIGHUP, &action, &old_action) < 0 ) {
        debug("sigaction error");
    }

    listenfd = open_listenfd(port);
    if(listenfd<0) {
        debug("open_listenfd error");
    }

    while(1) {
        connfdp = malloc(sizeof(int));
        if(connfdp==NULL) {
            debug("malloc error");
        }
        *connfdp = accept(listenfd, (SA *)&clientaddr, &clientlen);
        if(*connfdp<0) {
            debug("accept error");
        }
        int ret;
        if( (ret = pthread_create(&tid, NULL, pbx_client_service, connfdp))!=0 ) {
            debug("pthread_create error");
        }
    }

    fprintf(stderr, "You have to finish implementing main() "
	    "before the PBX server will function.\n");

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    debug("Shutting down PBX...");
    pbx_shutdown(pbx);
    debug("PBX server terminating");
    exit(status);
}

void sighup_handler() {
    terminate(EXIT_SUCCESS); // ???
}
