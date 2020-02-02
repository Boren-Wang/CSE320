#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "const.h"
#include "sequitur.h"
#include "debug.h"

/*
 * This file is a stub for code that I have provided for you, but you need
 * to "unlock" it.  When you successfully complete the decompression part
 * of the assignment, use it to decompress the file ../rsrc/sequitur.c.seq
 * and replace this stub with the output of the decompression.
 */

/**
 * Insert a new symbol after a specified symbol, handling any digram deletions
 * that result.
 *
 * @param this  The symbol after which the new symbol is to be inserted.
 * @param next  The symbol that is to be inserted.
 */
void insert_after(SYMBOL *this, SYMBOL *next) {
    // This is a non-functioning stub.
}

/**
 * Function to check for whether a just-created digram already exists somewhere.
 * If it does, then we have to restore the "no repeated digrams" condition
 * by replacing these digrams by a nonterminal.  If the pre-existing digram is
 * already the entire body of an existing rule, then there is no need to create
 * a new rule.  In that case, we just replace the just-created digram by a
 * nonterminal that refers to that existing rule.  Otherwise, we create a new
 * rule, with a new nonterminal at its head, and we use that new nonterminal
 * to replace the two existing instances of the digram.
 *
 * @param this  The first symbol of the digram to be checked.
 * @return  0 if no replacement was performed, nonzero otherwise.
 */
int check_digram(SYMBOL *this) {
    // This is a non-functioning stub.
    return 0;
}
