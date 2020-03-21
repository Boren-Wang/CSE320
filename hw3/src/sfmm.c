/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"

int free_list(size_t size);
sf_block* find_block(int size, int index);

void *sf_malloc(size_t size) {
    if(size==0){
        return NULL;
    }
    size+=8;
    int remainder = size%64;
    if(remainder>0){
        size = (size/64)*64+64;
    }
    int index = free_list(size);
    sf_block* bp = find_block(size, index);
    if(bp==NULL){

    } else {

    }

    return NULL;
}

int free_list(size_t size){
    size = size/64;
    if(size==1){
        return 0;
    } else if(size==2) {
        return 1;
    } else if(size==3) {
        return 2;
    } else if(size>3 && size<=5) {
        return 3;
    } else if(size>5 && size<=8) {
        return 4;
    } else if(size>8 && size<=13) {
        return 5;
    } else if(size>13 && size<=21) {
        return 6;
    } else if(size>21 && size<=34) {
        return 7;
    } else if(size>34) {
        return 8;
    } else {
        perror("Invalid size");
        return -1;
    }
}

sf_block* find_block(int size, int index){
    for(index; index<NUM_FREE_LISTS; index++){
        sf_block head = sf_free_list_heads[index];
        if(head.body.links.prev==head.body.links.next){
            continue;
        } else {
            sf_block cursor = head;
            while(cursor.body.links.next!=&head){
                cursor = *cursor.body.links.next;
                size_t block_size = cursor.header & BLOCK_SIZE_MASK;
                if(block_size>=size){
                    return &cursor;
                }
            }
        }
    }
    return NULL;
}

void sf_free(void *pp) {
    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    return NULL;
}

void *sf_memalign(size_t size, size_t align) {
    return NULL;
}
