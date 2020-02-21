#include <stdio.h>
#include <stdlib.h>

#include "const.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    // for(int i=1; i<argc; i++){
    //     char *s =  argv[i];
    //     printf("%s\n", s);
    // }
    // printf("%zu\n", sizeof(int));
    int ret;
    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    debug("Options: 0x%x", global_options); // ?
    if(global_options & 1)
        USAGE(*argv, EXIT_SUCCESS);
    if(global_options == 0x00000004){
        int count = decompress(stdin, stdout);
        // printf("This count is %d\n", count);
    }
    if((global_options & 0x2) == 2){
        // printf("Enter compress block!\n");
        int bszie = ((global_options >> 16) & 0xffff) * 1000;
        int count = compress(stdin, stdout, bszie);
        // printf("This count is %d\n", count);
    }

    return EXIT_SUCCESS;

}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
