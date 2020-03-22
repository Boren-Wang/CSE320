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
size_t getSize(sf_block* bp);

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
    if(bp==NULL){ // no large enough block found, grow the heap

    } else {
        if(getSize(bp)-size<64){ // leave splinter, do not split
            // modify the allocation status
            bp->header = bp->header | THIS_BLOCK_ALLOCATED;

        } else { // splitting
            int lowersize = size;
            int uppersize = getSize(bp)-size;
            sf_block* upperblock = bp+lowersize/sizeof(sf_block); // the pointer to the upper block

            // modify lower block
            bp->header = bp->header | lowersize; // modify the size for the lower block
            bp->header = bp->header | THIS_BLOCK_ALLOCATED; // modify the allocation status

            // modify upper block
            upperblock->header = uppersize; // set upper block size
            upperblock->header = upperblock->header | PREV_BLOCK_ALLOCATED; // prev block is allocated
            sf_block* nextblock = upperblock + uppersize/sizeof(sf_block); // the pointer to the next block after the upper block
            nextblock->prev_footer = upperblock->header; // set the footer for the upper block

            // add the upper block to a appropriate free list

        }
        return bp;
    }

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
    for(int i=index; i<NUM_FREE_LISTS; i++){
        sf_block head = sf_free_list_heads[i];
        if(head.body.links.prev==head.body.links.next){
            continue;
        } else {
            sf_block* cursor = &head;
            while(cursor->body.links.next!=&head){
                cursor = cursor->body.links.next;
                size_t block_size = cursor->header & BLOCK_SIZE_MASK;
                if(block_size>=size){
                    return cursor;
                }
            }
        }
    }
    return NULL;
}

size_t getSize(sf_block* bp){
    return bp->header & BLOCK_SIZE_MASK;
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
