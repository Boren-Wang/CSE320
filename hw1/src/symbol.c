#include "const.h"
#include "sequitur.h"

/*
 * Symbol management.
 *
 * The functions here manage a statically allocated array of SYMBOL structures,
 * together with a stack of "recycled" symbols.
 */

/*
 * Initialization of this global variable that could not be performed in the header file.
 */
int next_nonterminal_value = FIRST_NONTERMINAL;

SYMBOL* recycled_symbols = NULL;

/**
 * Initialize the symbols module.
 * Frees all symbols, setting num_symbols to 0, and resets next_nonterminal_value
 * to FIRST_NONTERMINAL;
 */
void init_symbols(void) {
    // To be implemented.
    for(int i=0; i<MAX_SYMBOLS; i++){
        symbol_storage->value = 0;
        symbol_storage->refcnt = 0;
        symbol_storage->rule = NULL;
        symbol_storage->next = NULL;
        symbol_storage->prev = NULL;
        symbol_storage->next = NULL;
        symbol_storage->prevr = NULL;
    }
    // symbol_storage = NULL;
    num_symbols = 0;
    next_nonterminal_value = FIRST_NONTERMINAL;
}

/**
 * Get a new symbol.
 *
 * @param value  The value to be used for the symbol.  Whether the symbol is a terminal
 * symbol or a non-terminal symbol is determined by its value: terminal symbols have
 * "small" values (i.e. < FIRST_NONTERMINAL), and nonterminal symbols have "large" values
 * (i.e. >= FIRST_NONTERMINAL).
 * @param rule  For a terminal symbol, this parameter should be NULL.  For a nonterminal
 * symbol, this parameter can be used to specify a rule having that nonterminal at its head.
 * In that case, the reference count of the rule is increased by one and a pointer to the rule
 * is stored in the symbol.  This parameter can also be NULL for a nonterminal symbol if the
 * associated rule is not currently known and will be assigned later.
 * @return  A pointer to the new symbol, whose value and rule fields have been initialized
 * according to the parameters passed, and with other fields zeroed.  If the symbol storage
 * is exhausted and a new symbol cannot be created, then a message is printed to stderr and
 * abort() is called.
 *
 * When this function is called, if there are any recycled symbols, then one of those is removed
 * from the recycling list and used to satisfy the request.
 * Otherwise, if there currently are no recycled symbols, then a new symbol is allocated from
 * the main symbol_storage array and the num_symbols variable is incremented to record the
 * allocation.
 */
SYMBOL *new_symbol(int value, SYMBOL *rule) {
    // To be implemented.
    // printf("Creating new symbol, value: %x\n", value & 0xffffffff);
    if(value<FIRST_NONTERMINAL){
        rule = NULL;
    }
    SYMBOL *new_symbol;
    if(recycled_symbols==NULL){
        if(num_symbols==MAX_SYMBOLS){
            fprintf(stderr, "The symbol storage is exhausted!\n");
            abort();
        }
        num_symbols++;
        new_symbol = symbol_storage+num_symbols-1;
        new_symbol->value = value;
        new_symbol->rule = rule;
    } else {
        new_symbol = recycled_symbols;
        recycled_symbols = recycled_symbols->next;
        new_symbol->next = NULL;
        new_symbol->value = value;
        new_symbol->rule = rule;
    }
    if(rule!=NULL){
            ref_rule(rule);
    }
    if(value<FIRST_NONTERMINAL){
        debug("New terminal symbol (%d)", value);
    } else {
        debug("New non-terminal symbol (%d)", value);
    }
    return new_symbol;
}

/**
 * Recycle a symbol that is no longer being used.
 *
 * @param s  The symbol to be recycled.  The caller must not use this symbol any more
 * once it has been recycled.
 *
 * Symbols being recycled are added to the recycled_symbols list, where they will
 * be made available for re-allocation by a subsequent call to new_symbol.
 * The recycled_symbols list is managed as a LIFO list (i.e. a stack), using the
 * next field of the SYMBOL structure to chain together the entries.
 */
void recycle_symbol(SYMBOL *s) {
    // To be implemented.
    if(recycled_symbols==NULL){
        recycled_symbols = s;
    } else {
        s->next = recycled_symbols;
        recycled_symbols = s;
    }
    recycled_symbols -> refcnt = 0;
    recycled_symbols -> rule = NULL;
    recycled_symbols -> next = NULL;
    recycled_symbols -> prev = NULL;
    recycled_symbols -> nextr = NULL;
    recycled_symbols -> prevr = NULL;
    debug("Recycle symbol (%d)", s->value);
}
