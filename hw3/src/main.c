#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_mem_init();

    // double* ptr = sf_malloc(sizeof(double));
    double* ptr = sf_malloc(10);

    // sf_show_blocks();
    // sf_show_free_lists();
    sf_show_heap();

    *ptr = 320320320e-320;

    // printf("%f\n", *ptr);

    // sf_free(ptr);

    // sf_show_blocks();
    // sf_show_free_lists();
    // sf_show_heap();

    sf_mem_fini();

    return EXIT_SUCCESS;
}
