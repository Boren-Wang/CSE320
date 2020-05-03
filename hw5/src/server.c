#include "csapp.h"

void *pbx_client_service(void *arg) {
    int connfd = *( (int*)arg );
    Pthread_detach(pthread_self());
    Free(arg);
    // service loop

    Close(connfd);
    return NULL;
}