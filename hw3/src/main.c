#include <stdio.h>
#include "sfmm.h"

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
    int* ip1 = sf_malloc(10);
    int* ip2 = sf_malloc(50);
    int* ip3 = sf_malloc(100);
    sf_show_heap();
    // printf("%p%p%p", ip1, ip2, ip3);
    // sf_free(ptr);
    sf_free(ip2);
    sf_show_heap();
    sf_free(ip1);
    sf_show_heap();
    sf_free(ip3);


    // sf_show_blocks();
    // sf_show_free_lists();
    sf_show_heap();

    sf_mem_fini();

    return EXIT_SUCCESS;
}
