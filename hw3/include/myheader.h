#include "sfmm.h"

void initializeFreelists();
int free_list(size_t size);
sf_block* find_block(size_t size, int index);
void addToFreelist(int index, sf_block* block);
void deleteFromFreelist(sf_block* block);

size_t getSize(sf_block* bp);
int isAlloc(sf_block* bp);
int isPrevAlloc(sf_block* bp);
void setSize(sf_block* bp, size_t size);
void setAlloc(sf_block* bp, size_t alloc);
void setPrevAlloc(sf_block* bp, size_t prevAlloc);

sf_block* getWildernessBlock();
void split(size_t size, sf_block* bp);

int validPointer(void *p);
sf_block* getNextBlock(sf_block* bp);
sf_block* getPrevBlock(sf_block* bp);
int prevBlockIsFree(sf_block* bp);
int isFree(sf_block* bp);
int isWildernessBlock(sf_block* bp);