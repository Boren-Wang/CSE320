#include <stdio.h>
#include "sfmm.h"
#include "myheader.h"

int main(int argc, char const *argv[]) {
    sf_mem_init();

    // double* ptr = sf_malloc(sizeof(double));
    // double* ptr = sf_malloc(10);
    // sf_show_heap();
    // ptr = sf_malloc(50);
    // sf_show_heap();
    // ptr = sf_malloc(100);
    // ptr = sf_malloc(100);
    // ptr = sf_malloc(100);
    // ptr = sf_malloc(3000);
    // sf_show_heap();
    // ptr = sf_malloc(10000);
    // sf_show_heap();
    // ptr = sf_malloc(5000);
    // ptr = sf_malloc(5000);
    // ptr = sf_malloc(5000);
    // ptr = sf_malloc(10000);
    // sf_show_heap();
    // ptr = sf_malloc(10000);
    // ptr = sf_malloc(10000);
    // ptr = sf_malloc(10);
    // sf_show_heap();

    // for(int i=0; i<10; i++){
    //     ptr = sf_malloc(10000);
    //     sf_show_heap();
    // }

    // sf_show_heap();

    // sf_show_blocks();
    // sf_show_free_lists();
    // sf_show_heap();

    // *ptr = 320320320e-320;

    // printf("%f\n", *ptr);
    // int* ip1 = sf_malloc(10);
    // int* ip2 = sf_malloc(50);
    // int* ip3 = sf_malloc(100);
    // sf_show_heap();
    // printf("%p%p%p", ip1, ip2, ip3);
    // sf_free(ptr);
    // sf_free(ip2);
    // sf_show_heap();
    // sf_free(ip1);
    // sf_show_heap();
    // sf_free(ip3);

    // void *x = sf_malloc(sizeof(double) * 8);
    // sf_show_heap();
    // void *y = sf_realloc(x, sizeof(int));
    // sf_show_heap();
    // printf("%p%p", x, y);

    // sf_show_blocks();
    // sf_show_free_lists();
    // sf_show_heap();

    // sf_malloc(sizeof(int));
    // sf_show_heap();
    // sf_memalign(1000, 4096);

    // sf_malloc(sizeof(int));
    // sf_memalign(3820, 4096);
    // sf_show_heap();

    // sf_malloc(8);
    // sf_memalign(1000, 4096);
    // sf_show_heap();

    // void* pp;
    // sf_block *bp;
    // sf_block block;
    // int res;
    // pp = sf_malloc(8);
    // sf_show_heap();
    // pp = (void*)(0x559dbb6382f8+8); // address of a block
    // bp = (sf_block*)pp;
    // sf_free(pp);
    // pp = (void*)(0x55a3354d0278+8);
    // pp = sf_mem_start();
    // printf("%p\n", pp);
    // sf_free(pp);
    // printf("%lu\n", bp->header);
    // pp = &bp->body.payload;
    // bp = (sf_block*)( (pp)-16 );
    // sf_block block = *bp;
    // validPointer(pp);

    // sf_malloc(8);
    // sf_show_heap();
    sf_mem_fini();

    return EXIT_SUCCESS;
}
