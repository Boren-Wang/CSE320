#include "const.h"
#include "sequitur.h"
#include "myheader.h"

/*
 * Digram hash table.
 *
 * Maps pairs of symbol values to first symbol of digram.
 * Uses open addressing with linear probing.
 * See, e.g. https://en.wikipedia.org/wiki/Open_addressing
 */

/**
 * Clear the digram hash table.
 */
void init_digram_hash(void) {
    // To be implemented.
    for(int i=0; i<MAX_DIGRAMS; i++){
        *(digram_table+i) = NULL;
    }
}

/**
 * Look up a digram in the hash table.
 *
 * @param v1  The symbol value of the first symbol of the digram.
 * @param v2  The symbol value of the second symbol of the digram.
 * @return  A pointer to a matching digram (i.e. one having the same two
 * symbol values) in the hash table, if there is one, otherwise NULL.
 */
SYMBOL *digram_get(int v1, int v2) {
    // To be implemented.
    // debug("Looking up digram: (%d, %d)", v1, v2);
    int hash = DIGRAM_HASH(v1, v2);
    SYMBOL **start = digram_table + hash;
    SYMBOL **end = digram_table + MAX_DIGRAMS - 1;

    for(SYMBOL** index=start; index<=end; index++){
        if((*index)==NULL){
            return NULL;
        } else if((*index)==TOMBSTONE) {
            continue;
        } else {
            SYMBOL s1 = **index;
            SYMBOL s2 = *((*index)->next);
            if(s1.value==v1 && s2.value==v2){
                return *index;
            }
        }
    }

    for(SYMBOL** index=digram_table; index<=start-1; index++){
        if((*index)==NULL){
            return NULL;
        } else if((*index)==TOMBSTONE) {
            continue;
        } else {
            SYMBOL s1 = **index;
            SYMBOL s2 = *((*index)->next);
            if(s1.value==v1 && s2.value==v2){
                return *index;
            }
        }
    }
    // debug("Not found!");
    return NULL;
}

/**
 * Delete a specified digram from the hash table.
 *
 * @param digram  The digram to be deleted.
 * @return 0 if the digram was found and deleted, -1 if the digram did
 * not exist in the table.
 *
 * Note that deletion in an open-addressed hash table requires that a
 * special "tombstone" value be left as a replacement for the value being
 * deleted.  Tombstones are treated as vacant for the purposes of insertion,
 * but as filled for the purpose of lookups.
 *
 * Note also that this function will only delete the specific digram that is
 * passed as the argument, not some other matching digram that happens
 * to be in the table.  The reason for this is that if we were to delete
 * some other digram, then somebody would have to be responsible for
 * recycling the symbols contained in it, and we do not have the information
 * at this point that would allow us to be able to decide whether it makes
 * sense to do it here.
 */
int digram_delete(SYMBOL *digram) {
    // To be implemented.
    int v1 = digram -> value;
    int v2 = digram->next->value;
    int hash = DIGRAM_HASH(v1, v2);
    SYMBOL **start = digram_table + hash;
    SYMBOL **end = digram_table + MAX_DIGRAMS - 1;

    for(SYMBOL** index=start; index<=end; index++){
        // if((*index)==NULL || (*index)==TOMBSTONE){
        //     continue;
        // } else {
        //     SYMBOL s1 = **index;
        //     SYMBOL s2 = *(*index+1);
        //     if(s1.value==v1 && s2.value==v2){
        //         (*index) = TOMBSTONE;
        //         return 0;
        //     }
        // }
        if((*index) == digram){
            (*index) = TOMBSTONE;
            // debug("Deleting digram: (%d, %d)", v1, v2);
            return 0;
        }
    }

    for(SYMBOL** index=digram_table; index<=start-1; index++){
        // if((*index)==NULL || (*index)==TOMBSTONE){
        //     *index = digram;
        //     return 0;
        // } else {
        //     SYMBOL s1 = **index;
        //     SYMBOL s2 = *(*index+1);
        //     if(s1.value==v1 && s2.value==v2){
        //         (*index) = TOMBSTONE;
        //         return 0;
        //     }
        // }
        if((*index) == digram){
            (*index) = TOMBSTONE;
            // debug("Deleting digram: (%d, %d)", v1, v2);
            return 0;
        }
    }
    // debug("Not found!");
    return -1; // The digram does not exist
}

/**
 * Attempt to insert a digram into the hash table.
 *
 * @param digram  The digram to be inserted.
 * @return  0 in case the digram did not previously exist in the table and
 * insertion was successful, 1 if a matching digram already existed in the
 * table and no change was made, and -1 in case of an error, such as the hash
 * table being full or the given digram not being well-formed.
 */
int digram_put(SYMBOL *digram) {
    // To be implemented.
    int v1 = digram -> value;
    // int v2 = (digram+1)->value;
    if(digram->next==NULL){
        fprintf(stderr, "Given digram is not well-formed!");
        abort();
    }
    int v2 = digram->next->value;
    int hash = DIGRAM_HASH(v1, v2);
    SYMBOL **start = digram_table + hash;
    SYMBOL **end = digram_table + MAX_DIGRAMS - 1;

    for(SYMBOL** index=start; index<=end; index++){
        if((*index)==NULL || (*index)==TOMBSTONE){
            *index = digram;
            // debug("Putting digram: (%d, %d)", v1, v2);
            return 0;
        } else {
            SYMBOL s1 = **index;
            SYMBOL s2 = *((*index)->next);
            if(s1.value==v1 && s2.value==v2){ // The digram already exists
                return 1;
            }
        }
    }

    for(SYMBOL** index=digram_table; index<=start-1; index++){
        if((*index)==NULL || (*index)==TOMBSTONE){
            *index = digram;
            // debug("Putting digram: (%d, %d)", v1, v2);
            return 0;
        } else {
            SYMBOL s1 = **index;
            SYMBOL s2 = *((*index)->next);
            if(s1.value==v1 && s2.value==v2){
                return 1;
            }
        }
    }
    return -1; // The hashtable is full
}
