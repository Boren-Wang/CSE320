#include <stdio.h>
#include <string.h>

#include "csapp.h"
#include "pbx.h"
#include "debug.h"

void *pbx_client_service(void *arg) {
    debug("start");
    int connfd = *( (int*)arg );
    Pthread_detach(pthread_self());
    Free(arg);

    // register the client file descriptor
    TU* tu = pbx_register(pbx, connfd);

    // service loop
    FILE* read = fdopen(connfd, "r");
    while(1) {
        debug("service loop");
        char c;
        char* line;
        line = malloc(10);
        if(line==NULL) {
            debug("malloc error");
        }
        int i = 0; // index
        size_t len = 10; // length of the line
        debug("about to read the line");
        while( ( c = fgetc(read))!=EOF && c!='\r' ) { // read the line
            debug("reading the line");
            if(i==len) {
                len += 10;
                line = realloc(line, len);
            }
            line[i] = c;
            i++;
        }
        debug("finish reading the line");
        if(c=='\r') {
            if( (c=fgetc(read)=='\n') ) {
                if(i==len) {
                    len += 10;
                    line = realloc(line, len);
                }
                line[i++] = '\0';
            } else if(c==EOF){
                // debug("EOF after \r");
                // break; //EOF
                if( (c=fgetc(read)==EOF) ) {
                    debug("EOF after \r");
                    break; //EOF
                }
            }
        }  else if(c==EOF) {
            // debug("EOF");
            // break; // EOF
            if( (c=fgetc(read)==EOF) ) {
                debug("EOF");
                break; // EOF
            }
        }
        debug("line is %s", line);

        // parse the line
        debug("start to parse the line");
        if( strcmp(line, "pickup")==0 ) {
            debug("pickup");
            int ret;
            if( (ret = tu_pickup(tu))==-1 ) {
                debug("tu_pickup error");
            }
        } else if( strcmp(line, "hangup")==0 ) {
            debug("hangup");
            int ret;
            if( (ret = tu_hangup(tu))==-1 ) {
                debug("tu_hangup error");
            }
        } else if( line[0]=='d' && line[1]=='i' && line[2]=='a' && line[3]=='l' && line[4]==' ' ) {
            debug("dial block starts");
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
                    i++;
                }
            }
            if(valid) {
                debug("valid number");
                int ret;
                if( (ret = tu_dial(tu, number))==-1 ) {
                    debug("tu_dial error");
                }
            } else {
                debug("invalid input: number contains non-digit");
            }
            debug("dial block ends");
        } else if( line[0]=='c' && line[1]=='h' && line[2]=='a' && line[3]=='t' ) {
            debug("chat block");
            char* chat;
            if(line[4]==' ') {
                chat = &line[5];
            } else {
                chat = &line[4];
            }
            int ret;
            if( (ret = tu_chat(tu, chat))==-1 ) {
                debug("tu_chat error");
            }
        } else { // invalid input
            debug("invalid input");
        }
        free(line);
    }
    pbx_unregister(pbx, tu);
    // fclose(read); // ???
    // Close(connfd); // ???
    return NULL;
}