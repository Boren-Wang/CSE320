/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "debug.h"
#include "sfmm.h"
#include "myheader.h"

static int freelistsInitialized = 0;
static int heapInitialized = 0;

void *sf_malloc(size_t size) {
    if(freelistsInitialized==0){
        initializeFreelists();
        freelistsInitialized = 1;
    }
    if(size==0){
        return NULL;
    }
    size+=8;
    int remainder = size%64;
    if(remainder>0){
        size = (size/64)*64+64;
        // printf("The size is %lu", size);
    }
    int index = free_list(size);
    sf_block* bp = find_block(size, index);
    if(bp==NULL){ // no large enough block found, grow the heap
        sf_block* wilderness = NULL;
        while(1){
            sf_block* new_page = sf_mem_grow();
            if(new_page==NULL){
                // printf("No memory\n");
                sf_errno = ENOMEM;
                return NULL;
            }
            if(heapInitialized==0){ // first allocation
                // printf("Initializing\n");
                // initialize prologue
                // printf("New page is %p\n", new_page);
                // printf("Mem_start is %p\n", sf_mem_start());
                // sf_block* prologue = new_page+(56/sizeof(sf_block)); // this is wrong!!!
                // printf("size of sf_block is %lu\n", sizeof(sf_block));
                sf_block* prologue = (sf_block*)( ((char*)new_page)+48 );
                // printf("Prologue is %p\n", prologue);
                // printf("Header is %lu\n", prologue->header);
                setSize(prologue, 64);
                setAlloc(prologue, 1);
                setPrevAlloc(prologue, 1);
                // printf("Header is now %lu\n", prologue->header);
                wilderness = getNextBlock(prologue);
                wilderness->prev_footer = prologue->header;
                setSize(wilderness, 3968);
                setAlloc(wilderness, 0);
                setPrevAlloc(wilderness, 1);
                sf_block* epilogue = getNextBlock(wilderness);
                epilogue->prev_footer = wilderness->header;
                setSize(epilogue, 0);
                setAlloc(epilogue, 1);
                setPrevAlloc(epilogue, 0);
                // add the wilderness block to the last free list
                addToFreelist(NUM_FREE_LISTS-1, wilderness);
                heapInitialized = 1;
            } else {
                // move epilogue
                sf_block* old_epilogue = (sf_block*)(((char*)new_page)-16);
                sf_block* new_epilogue = (sf_block*)(((char*)sf_mem_end())-16);
                *new_epilogue = *old_epilogue;
                // coalesce
                wilderness = getWildernessBlock();
                // sf_show_block(wilderness);
                // printf("\n");
                if(wilderness==NULL){
                    // printf("New wilderness block\n");
                    wilderness = (sf_block*)(((char*)new_page)-16);
                    setSize(wilderness, PAGE_SZ);
                    setAlloc(wilderness, 0);
                    setPrevAlloc(wilderness, 1);
                    getNextBlock(wilderness)->prev_footer = wilderness->header;
                    addToFreelist(NUM_FREE_LISTS-1, wilderness);
                } else {
                    setSize(wilderness, PAGE_SZ+getSize(wilderness));
                    getNextBlock(wilderness)->prev_footer = wilderness->header;
                    // printf("The size of the new wilderness block is: %lu\n", getSize(wilderness));
                    // printf("Size is %lu\n", size);
                }
            }
            if( getSize(wilderness) >= size ){ // the wilderness block is large enough
                break;
            } // if not large enough, continue growing the heap
        }
        // at this point, we have a large enough wilderness block
        split(size, wilderness);
        return wilderness->body.payload;
    } else {
        split(size, bp);
        return bp->body.payload;
    }
}

void initializeFreelists(){
    for(int i=0; i<NUM_FREE_LISTS; i++){
        sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
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
        // perror("Invalid size");
        return -1;
    }
}

sf_block* find_block(size_t size, int index){
    for(int i=index; i<NUM_FREE_LISTS; i++){
        sf_block* head = &sf_free_list_heads[i];
        if(head->body.links.prev==head->body.links.next && head->body.links.prev==head){
            continue;
        } else {
            sf_block* cursor = head;
            while(cursor->body.links.next!=head){
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

void addToFreelist(int index, sf_block* block){
    sf_block* dummy_head = &sf_free_list_heads[index];
    sf_block* head = dummy_head->body.links.next;
    dummy_head->body.links.next = block;
    head->body.links.prev = block;
    block->body.links.next = head;
    block->body.links.prev = dummy_head;
}

void deleteFromFreelist(sf_block* block){
    for(int i=0; i<NUM_FREE_LISTS; i++){
        sf_block* head = &sf_free_list_heads[i];
        if(head->body.links.prev==head->body.links.next && head->body.links.prev==head){
            // printf("Free list %d is empty\n", i);
            continue;
        } else {
            sf_block* cursor = head;
            while(cursor->body.links.next!=head){
                cursor = cursor->body.links.next;
                if(cursor == block){
                    sf_block* prev = cursor->body.links.prev;
                    sf_block* next = cursor->body.links.next;
                    prev->body.links.next = next;
                    next->body.links.prev = prev;
                }
            }
        }
    }
}

size_t getSize(sf_block* bp){
    return bp->header & BLOCK_SIZE_MASK;
}

int isAlloc(sf_block* bp){
    if( (bp->header&THIS_BLOCK_ALLOCATED)!=0 ){
        return 1;
    } else {
        return 0;
    }
}

int isPrevAlloc(sf_block* bp){
    if( (bp->header&PREV_BLOCK_ALLOCATED)!=0 ){
        return 1;
    } else {
        return 0;
    }
}

void setSize(sf_block* bp, size_t size){
    sf_header header = bp->header;
    size_t allocation = header & THIS_BLOCK_ALLOCATED;
    size_t prev_allocation = header & PREV_BLOCK_ALLOCATED;
    bp->header = size + allocation + prev_allocation;
}

void setAlloc(sf_block* bp, size_t alloc){ // 1: allocated, 0: freed
    if(alloc){
        alloc = 0x1;
    } else {
        alloc = 0x0;
    }
    size_t prevAlloc = bp->header & PREV_BLOCK_ALLOCATED;
    size_t size = bp->header & BLOCK_SIZE_MASK;
    bp->header = size | alloc | prevAlloc;
}

void setPrevAlloc(sf_block* bp, size_t prevAlloc){ // 1: allocated, 0: freed
    if(prevAlloc){
        prevAlloc = 0x2;
    } else {
        prevAlloc = 0x0;
    }
    size_t alloc = bp->header & THIS_BLOCK_ALLOCATED;
    size_t size = bp->header & BLOCK_SIZE_MASK;
    bp->header = size | alloc | prevAlloc;
}

sf_block* getWildernessBlock(){
    if(sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next!=&sf_free_list_heads[NUM_FREE_LISTS-1]){
        return sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next;
    } else {
        return NULL;
    }
}

int isWildernessBlock(sf_block* bp){
    // sf_show_block(bp);
    // printf("\n");
    // sf_show_block(getNextBlock(bp));
    // printf("\n");
    // sf_show_block(sf_mem_end());
    // printf("\n");
    if( (void*)getNextBlock(bp) == (void*)(((char*)sf_mem_end())-16) ){
        return 1;
    } else {
        return 0;
    }
}

void split(size_t size, sf_block* bp){ // after splitting, the lower block is allocated, and the upper block is free
    if(getSize(bp)-size<64){ // leave splinter, do not split
        // modify the allocation status
        setAlloc(bp, 1);
        deleteFromFreelist(bp);
    } else { // splitting
        size_t lowersize = size;
        size_t uppersize = getSize(bp)-size;
        sf_block* upperblock = (sf_block*)( ((char*)bp)+lowersize ); // the pointer to the upper block

        // modify lower block
        setSize(bp, lowersize); // modify the size for the lower block
        setAlloc(bp, 1); // modify the allocation status

        // delete lower block from its free list
        deleteFromFreelist(bp);

        // modify upper block
        setSize(upperblock, uppersize); // set upper block size
        setPrevAlloc(upperblock, 1); // prev block is allocated
        sf_block* nextblock = getNextBlock(upperblock); // the pointer to the next block after the upper block
        nextblock->prev_footer = upperblock->header; // set the footer for the upper block

        // add the upper block to a appropriate free list
        if(isWildernessBlock(upperblock)){
            addToFreelist(NUM_FREE_LISTS-1, upperblock);
        } else {
            int index = free_list(uppersize);
            addToFreelist(index, upperblock);
        }
    }
}

void sf_free(void *pp) {
    sf_block* bp = (sf_block*)( ((char*)pp)-16 );
    if(validPointer(pp)){
        // coalesce
        sf_block* new = bp;
        if(prevBlockIsFree(bp) & isFree(getNextBlock(bp))){ // if prev and next blocks are freed
            // printf("Prev and next blocks are free\n");
            deleteFromFreelist(getPrevBlock(bp)); // delete the prev block from its free list
            deleteFromFreelist(getNextBlock(bp));
            size_t size = getSize(getPrevBlock(bp)) + getSize(bp) + getSize(getNextBlock(bp));
            new = getPrevBlock(bp);
            setSize(new, size);
        } else if(prevBlockIsFree(bp)) { // if prev block is freed
            // printf("Prev block is free\n");
            deleteFromFreelist(getPrevBlock(bp));
            size_t size = getSize(getPrevBlock(bp)) + getSize(bp);
            new = getPrevBlock(bp);
            setSize(new, size);
        } else if(isFree(getNextBlock(bp))) { // if next block is freed
            // printf("Next block is free\n");
            deleteFromFreelist(getNextBlock(bp));
            size_t size = getSize(bp) + getSize(getNextBlock(bp));
            new = bp;
            setSize(new, size);
        }
        setAlloc(new, 0);
        getNextBlock(new)->prev_footer = new->header;
        setPrevAlloc(getNextBlock(new), 0);
        if(isWildernessBlock(new)){ // if the new block is the wilderness block
            sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next = new;
            sf_free_list_heads[NUM_FREE_LISTS-1].body.links.prev = new;
            new->body.links.next = &sf_free_list_heads[NUM_FREE_LISTS-1];
            new->body.links.prev = &sf_free_list_heads[NUM_FREE_LISTS-1];
        } else { // add the coalesced block to appropriate free list
            int index = free_list(getSize(new));
            addToFreelist(index, new);
        }
    } else {
        abort();
    }
}

int validPointer(void *pp){ // need to test this function!!!
    if(pp==NULL){
        // printf("The pointer is NULL\n");
        return 0;
    }
    unsigned long p = (unsigned long)pp;
    if( p % 64 != 0 ){
        // printf("The pointer is not aligned to a 64-byte boundary\n");
        return 0;
    }
    if( pp-8 < sf_mem_start()+56+64 ){
        // printf("The header of the block is before the end of the prologue.\n");
        return 0;
    }
    sf_block* bp = (sf_block*)( ((char*)pp)-16 );
    sf_block* next = getNextBlock(bp);
    if( (void*)&(next->prev_footer) >  sf_mem_end()-8 ){
        // printf("The footer of the block is after the beginning of the epilogue.\n");
        return 0;
    }
    if( (bp->header & THIS_BLOCK_ALLOCATED) == 0 ){ // if the block's allocated bit is 0 (free block)
        // printf("The allocated bit in the header is 0\n");
        return 0;
    }
    // if( prevBlockIsFree(&block) && (block.prev_footer&THIS_BLOCK_ALLOCATED)!=0 ){
    if( prevBlockIsFree(bp) && ( (getPrevBlock(bp)->header) & THIS_BLOCK_ALLOCATED)!=0 ){
        // printf("The prev_alloc field is 0, indicating that the previous block is free, but the alloc field of the previous block header is not 0.\n");
        return 0;
    }
    return 1;
}

sf_block* getNextBlock(sf_block* bp){
    size_t size = getSize(bp);
    sf_block* next = bp+size/sizeof(sf_block);
    return next;
}

sf_block* getPrevBlock(sf_block* bp){
    size_t size = bp->prev_footer&BLOCK_SIZE_MASK;
    sf_block* prev = bp-size/sizeof(sf_block);
    return prev;
}

int prevBlockIsFree(sf_block* bp){
    if( (bp->header & PREV_BLOCK_ALLOCATED)==0 ){ // free
        return 1;
    } else { // allocated
        return 0;
    }
}

int isFree(sf_block* bp){
    if( (bp->header & THIS_BLOCK_ALLOCATED) == 0 ){
        return 1;
    } else {
        return 0;
    }
}

void *sf_realloc(void *pp, size_t rsize) {
    if(validPointer(pp)){
        if(rsize==0){
            sf_free(pp);
            return NULL;
        }
        size_t rbsize = rsize+8; // The size of the new block for the reallocation
        int remainder = rbsize%64;
        if(remainder>0){
            rbsize = (rbsize/64)*64+64;
            // printf("The size is %lu", size);
        }
        sf_block* bp = (sf_block*)( ((char*)pp)-16 );
        size_t bsize = getSize(bp); // The size of the original block
        size_t size = getSize(bp)-8; // The size of the payload of the original block
        if(rbsize>bsize){ // if reallocating to a larger size
            void* new = sf_malloc(rsize);
            if(new==NULL){
                sf_errno = ENOMEM;
                return NULL;
            }
            memcpy(new, pp, size);
            sf_free(pp);
            return new;
        } else if (rbsize<bsize) { // if reallocating to a smaller size
            if( (bsize-rbsize)<64 ){ // do not split the block because of splinter
                return pp;
            } else { // split the block
                size_t lowersize = rbsize;
                size_t uppersize = bsize - rbsize;
                sf_block* upperblock = (sf_block*)( ((char*)bp)+lowersize ); // the pointer to the upper block

                // modify lower block
                setSize(bp, lowersize); // modify the size for the lower block
                setAlloc(bp, 1); // modify the allocation status

                // modify upper block
                setSize(upperblock, uppersize); // set upper block size
                setPrevAlloc(upperblock, 1); // prev block is allocated
                // setAlloc(upperblock, 0); // upper block is free
                // sf_block* nextblock = getNextBlock(upperblock); // the pointer to the next block after the upper block
                // nextblock->prev_footer = upperblock->header; // set the footer for the upper block

                // // add the upper block to a appropriate free list
                // if(isWildernessBlock(upperblock)){
                //     addToFreelist(NUM_FREE_LISTS-1, upperblock);
                // } else {
                //     int index = free_list(uppersize);
                //     addToFreelist(index, upperblock);
                // }
                setAlloc(upperblock, 1); // set upperblock's allocation bit to be 1 so that can use sf_free to free it
                sf_free(upperblock->body.payload);
                return pp;
            }
        } else { // if reallocating to the same size
            return pp;
        }
    } else { // invalid pointer
        sf_errno = EINVAL;
        abort();
    }
}

void *sf_memalign(size_t size, size_t align) {
    if( align<64 || (align & (align-1))!=0 ){
        sf_errno = EINVAL;
        return NULL;
    }
    size_t bsize = size+align+64; // and header size and padding size, which will be added during malloc
    void* pp = sf_malloc(bsize);
    if(pp==NULL){
        return NULL;
    }
    unsigned long p = (unsigned long) pp;
    sf_block* block = (sf_block*)(((char*)pp)-16); // block returned by malloc
    size_t blockSize = getSize(block);
    sf_block* newBlock;
    // sf_show_heap();
    if( (p & (align-1)) == 0 ){ // payload address is aligned with align
        // printf("Payload address is aligned with align\n");
        // post-splitting and freeing
        newBlock = (sf_block*)(((char*)pp)-16); // block to be return by this memalign
        size_t newBlockSize = size+8;
        int remainder = newBlockSize%64;
        if(remainder>0){
            newBlockSize = (newBlockSize/64)*64+64;
            // printf("The size is %lu", size);
        }
        split(newBlockSize, newBlock);
    } else { // if not aligned
        // printf("not aligned\n");
        size_t preBlockSize = 0;
        while( (p & (align-1)) != 0 ){
            p+=1;
            preBlockSize+=1;
        }
        pp = (void*)p;

        newBlock = (sf_block*)(((char*)pp)-16); // this is the block after pre-splitting
        size_t newBlockSize = blockSize-preBlockSize; // this is the size for the block after pre-splitting
        setSize(newBlock, newBlockSize);
        setPrevAlloc(newBlock, 1);
        setAlloc(newBlock, 1); // to prevent from being coalesced

        // pre-splitting and freeing
        setSize(block, preBlockSize);
        // sf_show_heap();
        sf_free(block->body.payload);
        // sf_show_heap();

        // post-splitting and freeing
        size_t midBlockSize = size+8; // size for the mid block after pre and post splitting
        int remainder = midBlockSize%64;
        if(remainder>0){
            midBlockSize = (midBlockSize/64)*64+64;
            // printf("The size is %lu", size);
        }
        // sf_show_heap();
        split(midBlockSize, newBlock);
    }
    sf_block* postBlock = getNextBlock(newBlock);
    if(isWildernessBlock(getNextBlock(postBlock))){ // coalesce post block with wilderness block
        deleteFromFreelist(postBlock);
        deleteFromFreelist(getNextBlock(postBlock));
        size_t size = getSize(postBlock) + getSize(getNextBlock(postBlock));
        setSize(postBlock, size);
        getNextBlock(postBlock)->prev_footer = postBlock->header;
        sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next = postBlock;
        sf_free_list_heads[NUM_FREE_LISTS-1].body.links.prev = postBlock;
        postBlock->body.links.next = &sf_free_list_heads[NUM_FREE_LISTS-1];
        postBlock->body.links.prev = &sf_free_list_heads[NUM_FREE_LISTS-1];
    }
    // sf_show_heap();
    return pp;
}
