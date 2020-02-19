#include "const.h"
#include "sequitur.h"
#include "debug.h"
#include "myheader.h"
#include <stdlib.h>

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the pathname of the current file or directory
 * as well as other data have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */

/**
 * Main compression function.
 * Reads a sequence of bytes from a specified input stream, segments the
 * input data into blocks of a specified maximum number of bytes,
 * uses the Sequitur algorithm to compress each block of input to a list
 * of rules, and outputs the resulting compressed data transmission to a
 * specified output stream in the format detailed in the header files and
 * assignment handout.  The output stream is flushed once the transmission
 * is complete.
 *
 * The maximum number of bytes of uncompressed data represented by each
 * block of the compressed transmission is limited to the specified value
 * "bsize".  Each compressed block except for the last one represents exactly
 * "bsize" bytes of uncompressed data and the last compressed block represents
 * at most "bsize" bytes.
 *
 * @param in  The stream from which input is to be read.
 * @param out  The stream to which the block is to be written.
 * @param bsize  The maximum number of bytes read per block.
 * @return  The number of bytes written, in case of success,
 * otherwise EOF.
 */
int compress(FILE *in, FILE *out, int bsize) {
    // To be implemented.
    return EOF;
}

int toUTF8(int bytes, int bytec){
    // if(bytec == 1){
    //     return bytes;
    // } else
    if(bytec==2){
        int left = bytes & 0x1f00;
        left = left >> 2;
        int right = bytes & 0x003f;
        return left + right;
    } else if(bytec == 3){
        int left = bytes & 0xf0000;
        left = left >> 4;
        int middle = bytes & 0x3f00;
        middle = middle >> 2;
        int right = bytes & 0x003f;
        return left + middle + right;
    } else if(bytec == 4){
        int lleft = bytes & 0x7000000;
        lleft = lleft >> 6;
        int left = bytes & 0x3f0000;
        left = left >> 4;
        int right = bytes & 0x3f00;
        right = right >> 2;
        int rright = bytes & 0x3f;
        return lleft + left + right + rright;
    } else {
        return -1; // error
    }
}

void recursive_print(SYMBOL *head, FILE *out, int* countp, SYMBOL *referenced){
    // printf("Recursion starts...");
    // printf("The value of the main symbol is %x\n", (main_rule->value) & 0xffffffff);
    if(referenced!=NULL && head->value==referenced->value){
        return;
    }
    if(IS_TERMINAL(head)){
        // printf("The value of the terminal symbol is %x\n", (head->value) & 0xffffffff);
        fputc((char)head->value, out);
        *countp += 1;
        if(head->next==main_rule){
            // printf("The value of the symbol is %x\n", (head->next->value) & 0xffffffff);
            return;
        }
        recursive_print(head->next, out, countp, referenced);
    } else {
        // printf("The value of the non terminal symbol is %x\n", (head->value) & 0xffffffff);
        // printf("First body symbol's value is %x\n", ((*(rule_map+(head->value)))->next->value) & 0xffffffff);
        recursive_print((*(rule_map+(head->value)))->next, out, countp, *(rule_map+(head->value)));
        if(head->next==main_rule){
            printf("The value of the symbol is %x\n", (head->next->value) & 0xffffffff);
            return;
        }
        recursive_print(head->next, out, countp, referenced);
    }
}

void print_array(){
    // for(int i=0; i<SYMBOL_VALUE_MAX; i++){
    //     if(*(rule_map+i)!=NULL){
    //         printf("%x\n", ((*(rule_map+i))->value)&0xffffffff);
    //     }
    // }
    // for(int i=0; i<num_symbols; i++){
    //     printf("%x\n", ((symbol_storage+i)->value) & 0xffffffff);
    // }
    // printf("%x\n", ((*(rule_map+0x109))->value) & 0xffffffff);
    // printf("%x\n", ((*(rule_map+0x109))->value) & 0xffffffff);
    // printf("%x\n", ((*(rule_map+0x10a))->value) & 0xffffffff);
    // printf("%x\n", ((*(rule_map+0x109))->next->value) & 0xffffffff);
    // printf("%x\n", ((*(rule_map+0x109))->next->next->value) & 0xffffffff);
    // SYMBOL* rule = main_rule;
    // while(rule->nextr!=NULL){
    //     printf("The rule is %x\n", (rule->value) & 0xffffffff);
    //     rule = rule->nextr;
    // }
}

/**
 * Main decompression function.
 * Reads a compressed data transmission from an input stream, expands it,
 * and and writes the resulting decompressed data to an output stream.
 * The output stream is flushed once writing is complete.
 *
 * @param in  The stream from which the compressed block is to be read.
 * @param out  The stream to which the uncompressed data is to be written.
 * @return  The number of bytes written, in case of success, otherwise EOF.
 */
int decompress(FILE *in, FILE *out) {
    // To be implemented.
    int count = 0;
    int c = fgetc(in);
    // printf("The first byte is %x\n", c & 0xff);
    // printf("The first byte is %d\n", c);

    do{ // new transmission
        c = fgetc(in);
        if(c==0x83){ // new block
            // printf("The first byte of a new block is %x\n", c & 0xff);
            init_symbols();
            init_rules();
            do{
                c = fgetc(in);
                int byte1 = c;
                int v = 0;
                // int bytes = 0;
                // int bytec=0;
                // int h = (byte1 & 0xe0) >> 4;
                // printf("The head is %d\n", h);
                // printf("Decimal Hexidecimal Comparison: %d\n", 12==0xc);
                if(((byte1 & 0xe0) >> 4) == 0xc){ // two bytes
                    int byte2 = fgetc(in);
                    // bytec = 2;
                    // printf("Bytes is %x\n", ((byte1<<8)+byte2) & 0xffff);
                    v = toUTF8((byte1<<8)+byte2, 2);
                } else if(((byte1 & 0xf0) >> 4) == 0xe){ // three bytes
                    int byte2 = fgetc(in);
                    int byte3 = fgetc(in);
                    // bytec = 3;
                    v = toUTF8((byte1<<16)+(byte2<<8)+byte3, 3);
                } else if(((byte1 & 0xf0) >> 4) == 0xf){ // four bytes
                    int byte2 = fgetc(in);
                    int byte3 = fgetc(in);
                    int byte4 = fgetc(in);
                    // bytec = 4;
                    v = toUTF8((byte1<<24)+(byte2<<16)+(byte3<<8)+byte4, 4);
                } else {
                    printf("The byte is %x\n", byte1 & 0xff);
                }
                SYMBOL* head= new_rule(v);
                // printf("The value of the head of the new rule is %x, and the count of bytes is %d\n", v & 0xffffffff, bytec);
                c = fgetc(in);

                while(c!=0x85 && c!=0x84){
                    // printf("Forming rule body...\n");
                    byte1 = c;
                    // printf("The byte is %x\n", byte1 & 0xff);
                    v = 0;
                    // bytes = 0;
                    // bytec = 0;
                    if((byte1 & 0xe0) >> 4 == 0xc){ // two bytes
                        int byte2 = fgetc(in);
                        // bytes = (byte1<<8)+byte2;
                        // bytec = 2;
                        v = toUTF8((byte1<<8)+byte2, 2);
                    } else if((byte1 & 0xf0) >> 4 == 0xe){ // three bytes
                        int byte2 = fgetc(in);
                        int byte3 = fgetc(in);
                        // bytes = (byte1<<16)+(byte2<<8)+byte3;
                        // bytec = 3;
                        v = toUTF8((byte1<<16)+(byte2<<8)+byte3, 3);
                    } else if((byte1 & 0xf0) >> 4 == 0xf){ // four bytes
                        int byte2 = fgetc(in);
                        int byte3 = fgetc(in);
                        int byte4 = fgetc(in);
                        // bytes = (byte1<<24)+(byte2<<16)+(byte3<<8)+byte4;
                        // bytec = 4;
                        v = toUTF8((byte1<<24)+(byte2<<16)+(byte3<<8)+byte4, 4);
                    } else {
                        v = byte1;
                        // bytes = byte1;
                        // bytec = 1;
                    }
                    // printf("The bytes is %x, the value of the new symbol is %x, and the count of bytes is %d\n", bytes, v & 0xffffffff, bytec);
                    if(v>0x7f){ // if non terminal
                        // printf("Adding non terminal symbol\n");
                        SYMBOL *rule = *(rule_map+v); // rule can be null
                        // if(rule!=NULL){
                        //     printf("The rule's value is %x\n", (rule->value)&0xffffffff);
                        // }
                        // printf("New symbol value is %x\n", v&0xffffffff);
                        // printf("Head is %x\n", (head->value) & 0xffffffff);
                        add_symbol(head, new_symbol(v, rule));
                    } else {
                        // printf("Adding terminal symbol\n");
                        // printf("New symbol value is %x\n", v&0xffffffff);
                        // printf("Head is %x\n", (head->value) & 0xffffffff);
                        add_symbol(head, new_symbol(v, NULL));
                    }
                    c = fgetc(in);
                }
                add_rule(head);
            }while(c!=0x84);
            // printf("EOB-Printing...");
            recursive_print(main_rule->next, out, &count, NULL);
            fflush(out);
            print_array();
            c = fgetc(in);
        } else {
            printf("Enter EOF!");
            return EOF;
        }
    }while(c!=0x82);
    return count;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv)
{
    if(argc==1){ // -s
        return -1;
    }

    if(**(argv+1)=='-' && *(*(argv+1)+1)=='h'){ // s -h
        global_options =  0x1;
        return 0;
    }else if(**(argv+1)=='-' && *(*(argv+1)+1)=='c'){ // s -c
        if(argc==2){ // -c
            global_options = 0x04000002;
            return 0;
        }else if(**(argv+2)=='-' && *(*(argv+2)+1)=='b'){ // s -c -b
            // if(argc>3){
            if(argc==4){
                char* cp = *(argv+3);
                int size = 0;
                while(*cp!='\0'){
                    char c = *cp;
                    if(c<'0' || c>'9'){
                        return -1;
                    } else {
                        size*=10;
                        size+=(*cp-'0');
                    }
                    cp++;
                }
                if(size>-1 && size<1025){
                    printf("This size of the block is %d", size);
                    size = size<<16;
                    global_options = size + 0x2;
                    return 0;
                } else {
                    return -1;
                }
            }else{
                return -1;
            }
        }else{
            return -1;
        }

    }else if(**(argv+1)=='-' && *(*(argv+1)+1)=='d'){
        if(argc==2){
            global_options= 0x00000004;
            return 0;
        }else{
            return -1;
        }
    }else{
        return -1;
    }
}
