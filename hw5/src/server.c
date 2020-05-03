#include <stdio.h>
#include <string.h>

#include "csapp.h"
#include "pbx.h"
#include "debug.h"

void *pbx_client_service(void *arg) {
    int connfd = *( (int*)arg );
    Pthread_detach(pthread_self());
    Free(arg);

    // register the client file descriptor
    TU* tu = pbx_register(pbx, connfd);

    // service loop
    FILE* read = fdopen(connfd, "r");
    while(1) {
        char c;
        char* line;
        line = malloc(10);
        int i = 0; // index
        size_t len = 10; // length of the line
        while( ( c = fgetc(read))!=EOF || c!='\r' ) { // read the line
            if(i==len) {
                len += 10;
                line = realloc(line, len);
            }
            line[i] = c;
            i++;
        }
        if(c=='\r') {
            if( (c=fgetc(read)=='\n') ) {
                if(i==len) {
                    len += 10;
                    line = realloc(line, len);
                }
                line[i++] = '\0';
            }
        }

        // parse the line
        if( strcmp(line, "pickup")==0 ) {
            int ret;
            if( (ret = tu_pickup(tu))==-1 ) {
                debug("tu_pickup error");
            }
        } else if( strcmp(line, "hangup")==0 ) {
            int ret;
            if( (ret = tu_hangup(tu))==-1 ) {
                debug("tu_hangup error");
            }
        } else if( line[0]=='d' && line[1]=='i' && line[2]=='a' && line[3]=='l' && line[4]==' ' ) {
            i = 5;
            int number = 0;
            int valid = 1;
            while(line[i]!='\0') {
                c = line[i];
                if( c<'0' || c>'9' ){ // invalid input: number contains non-digit
                    valid = 0;
                    break; // ???
                } else {
                    number *= 10;
                    number += (c-'0');
                }
            }
            if(valid) {
                int ret;
                if( (ret = tu_dial(tu, number))==-1 ) {
                    debug("tu_dial error");
                }
            } else {
                debug("invalid input: number contains non-digit");
            }
        } else if( line[0]=='c' && line[1]=='h' && line[2]=='a' && line[3]=='t' && line[4]==' ' ) {
            char* chat = &line[5];
            int ret;
            if( (ret = tu_chat(tu, chat))==-1 ) {
                debug("tu_chat error");
            }
        } else { // invalid input
            debug("invalid input");
        }
        free(line);
    }

    Close(connfd);
    return NULL;
}