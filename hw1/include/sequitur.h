/*
 * DO NOT MODIFY THE CONTENTS OF THIS FILE.
 * IT WILL BE REPLACED DURING GRADING
 */
#ifndef SEQUITUR_H
#define SEQUITUR_H

#include <stdlib.h>
#include <stdio.h>

#include "debug.h"

/*
 * SYMBOLS
 *
 * The SYMBOL type defined below is the basic object that we will use in the
 * representation of symbols and rules.  This type is defined as a C structure
 * "struct symbol".  Each symbol has a "value" field, which determines the type
 * of the symbol (either terminal or nonterminal) and which uniquely identifies
 * that symbol.  "Small" values (i.e. < FIRST_NONTERMINAL) are used for terminal
 * symbols and "large" values (i.e. >= FIRST_NONTERMINAL) are used for nonterminal
 * symbols.  Terminal symbols correspond directly to individual bytes of data in
 * a string.  The value of a terminal symbol is simply the value of the corresponding
 * data byte, regarded as an unsigned integer in the range [0, FIRST_NONTERMINAL).
 * For example, the three-letter sequence `ABC` would be encoded in ASCII or UTF-8
 * as a sequence of three bytes with values 61, 62, 63.  We would represent this
 * sequence by a list of three SYMBOL structures having values 61, 62, and, 63.
 * Nonterminal symbols do not directly correspond to data bytes in a sequence being
 * compressed or decompressed, and the value of a nonterminal symbol has no particular
 * significance other than to distinguish one particular nonterminal symbol from another.
 *
 * Each symbol also has some pointer fields, which we will use to chain symbols together
 * into lists.  The "next" and "prev" fields will be used to chain symbols together
 * as a doubly linked list to form the body of a rule.  The "nextr" and "prevr" fields
 * will be used to link the heads of rules (which will be represented by nonterminal
 * symbols) into a doubly linked list.  There is also a "refcnt" field, which is used
 * by the compression algorithm to maintain a count of the number of times a rule has
 * been used.  Refer to the assignment document for further discussion on the use of these
 * various fields.
 */
typedef struct symbol {
    unsigned int value;        // The value that uniquely identifies the symbol.
    unsigned int refcnt;       // Reference count if symbol is head of a rule, otherwise 0
    struct symbol *rule;       // NULL for terminal, non-NULL for nonterminal or sentinel
    struct symbol *next;       // Next symbol in rule body (or the sentinel, in case of last symbol)
    struct symbol *prev;       // Previous symbol in rule body (or the sentinel, in case of first symbol)
    struct symbol *nextr;      // If sentinel, next rule in list of all rules.
    struct symbol *prevr;      // If sentinel, previous rule in list of all rules.
} SYMBOL;

/* The first symbol value that is used for nonterminal symbols. */
#define FIRST_NONTERMINAL 256

/* The following macros are used to inspect a symbol to determine what type it is. */
#define IS_TERMINAL(s) ((s)->value < FIRST_NONTERMINAL)
#define IS_NONTERMINAL(s) (!IS_TERMINAL(s))
#define IS_RULE_HEAD(s) ((s)->rule == (s))

/*
 * The following counter is used to allocate fresh values when new nonterminal symbols
 * need to be created.  It is initialized elsewhere, because to initialize it here
 * would result in a "multiple definition" link error, due to the inclusion of this
 * header in multiple source files.
 */
extern int next_nonterminal_value /* = FIRST_NONTERMINAL */;

/*
 * We will not use general-purpose dynamic storage allocation (i.e. "malloc").
 * Instead, we statically declare an array of a fixed number of SYMBOL structures.
 * We will keep track of the number of such structures that are in use, and when
 * we need another one, we will use the first unused one.  This will suffice for
 * the decompression algorithm, where we only ever allocate symbols and never
 * free them.  The compression algorithm, on the other hand, does free symbols
 * from time to time, and in order to avoid running out of symbols unnecessarily
 * we will implement a "recyling facility" for symbols that are not currently
 * being used.
 */

/* The total number of pre-allocated symbols that we have at our disposal. */
#define MAX_SYMBOLS 1000000

/* The maximum number of nonterminal symbols (limited by 2^21 Unicode code points). */
#define SYMBOL_VALUE_MAX (1 << 21)

/* Statically allocated storage for symbols (definition is in const.h). */
extern SYMBOL symbol_storage[/*MAX_SYMBOLS*/];

/* Total number of symbols that have been allocated from symbol_storage. */
extern int num_symbols;

/* Given a pointer to a symbol, obtain the index of the symbol in the symbol_storage array. */
#define SYMBOL_INDEX(s) ((s) - symbol_storage)

/*
 * RULES
 *
 * We represent a grammar rule by a circular, doubly linked list of the symbols
 * in the body of the rule, together with an additional "sentinel" symbol,
 * which is a nonterminal symbol that represents the head of the rule.
 * The sentinel is linked between the first and last symbol in the body of the
 * rule, resulting in the following (doubly linked) configuration:
 *
 *    H <-> B1 <-> B2 <-> ... <-> Bn
 *    ^                           ^
 *    |                           |
 *    +---------------------------+
 *
 * The "next" and "prev" fields of the SYMBOL structure are used to chain symbols
 * together into a rule.  We refer to a rule using a pointer to the sentinel node H.
 * Sentinel nodes are distinguishable from other nodes by virtue of their having
 * a non-NULL "rule" field that points back to the sentinel node itself.
 *
 * We also link rule heads together into a circular, doubly linked list of all rules,
 * using the "nextr" and "prevr" fields of the SYMBOL structure.  This list does not
 * use a sentinel node, but the global variable "main_rule" is used to point to a
 * distinguished "main rule" in the list, from which the other rules can be accessed:
 *
 *    main_rule -> H1 <-> H2 <-> H3 <-> ... <-> Hn
 *                  ^                           ^
 *                  |                           |
 *                  +---------------------------+
 */

/*
 * The following variable (defined in const.h) points to the "main rule".
 * Note that when the first rule is assigned to it, the "nextr" and "prevr" fields
 * of that rule must be initialized to point back to the rule itself, in order
 * to properly represent a circular, doubly linked list with one element in it.
 */
extern SYMBOL *main_rule;

/*
 * DIGRAMS
 *
 * A "digram" is a list of two symbols, which we will refer to by a pointer to its
 * first symbol.In general, digrams will occur within longer lists, so we don't assume
 * anything in particular about the "prev" pointer of the first symbol in a digram or
 * the "next" pointer of the second symbol in a digram.
 *
 * The compression algorithm will need to keep track of the set of digrams that
 * currently exist in the rules being constructed.  For that purpose, we will use
 * a hash table.  For simplicity, and to avoid the need to be able to dynamically allocate
 * additional data structures for use in constructing the table, we will use an
 * "open-addressed" hash table, which simply consists of an array, each of whose entries
 * can be set to point to a digram currently in the table.  Completely unused entries
 * in the hash table have value NULL.  Deletions in an open-addressed hash table have
 * to be handled by leaving a "tombstone" (distinguishable from NULL) in place of the
 * deleted entry.  We define a special value TOMBSTONE for this purpose.
 */

/* The size of the digram hash table, which must be a prime number. */
#define MAX_DIGRAMS 999983

/* Definition of the value to be used as a "tombstone" for deleted entries. */
#define TOMBSTONE ((SYMBOL *)-1)

/*
 * Statically allocated storage (the actual definition is in const.h) for the digram
 * hash table, which maps pairs of symbol values to digrams.
 */
extern SYMBOL *digram_table[/*MAX_DIGRAMS*/];

/*
 * Digram hash function: takes the two symbols of a digram and returns an
 * index into the hash table.  This index will serve as the starting point for
 * a "linear probing" search for a matching digram.  For further information on
 * open-addressed hash tables, linear probing, and deletion using tombstones,
 * refer to your favorite Data Structures book or to
 * https://en.wikipedia.org/wiki/Open_addressing
 */
#define DIGRAM_HASH(v1, v2) (((v1)*37 + (v2)) % MAX_DIGRAMS)

/*
 * FORMAT OF A COMPRESSED DATA TRANSMISSION
 *
 * A compressed data transmission begins with a "start of transmission" (SOT) mark,
 * which is a single byte having hexadecimal value 0x81, and it ends with an
 * "end of transmission" (EOT) mark, which is a single byte having hexadecimal
 * value 0x82.  Between these two marks is a (possibly empty) sequence of "blocks".
 * Each block begins with a "start of block" (SOB) mark, which is a single byte
 * having hexadecimal value 0x83, and it ends with an "end of block" (EOB) mark,
 * which is a single byte having hexadecimal value 0x84.  Between these two marks
 * is a nonempty sequence of "rules".  Rules are separated by "rule delimiter" (RD)
 * marks, which are single bytes having hexadecimal value 0x85.
 * Each rule consists of a sequence of three or more symbols, where each symbol
 * is a Unicode code point (in the valid Unicode code space of U+0000 to U+10FFFF)
 * encoded as a sequence of one to four bytes using UTF-8 encoding.  The first symbol
 * is the head of the rule and the remaining symbols constitute the body of the rule.
 * Symbols with values in the range U+0000 to U+00FF are terminal symbols, and must
 * not occur as the head of a rule.  Other symbols are nonterminal symbols.
 *
 * Note that no valid UTF-8 encoding can begin with a "continuation byte",
 * which is one whose value has the form 10xxxxxx in binary.  Each of the special
 * marks defined above has a value of such a form, so they cannot be confused with
 * a symbol in the body of a rule. For further information on UTF-8 encoding,
 * refer to https://en.wikipedia.org/wiki/UTF-8
 *
 * For a complete example of a short compressed data transmission, refer to the
 * assignment handout.
 */

/*
 * The following functions, which are part of the core implementation of the
 * Sequitur algorithm, will have to be called by your "compress" function.
 * You will be given code for these functions (actually, you will be able to
 * "unlock" the code once you successfully complete the decompression part of
 * the assignment) and do not have to implement them.
 * Stubs for these functions, with specifications, are located in file sequitur.c.
 */
void insert_after(SYMBOL *old, SYMBOL *new);
int check_digram(SYMBOL *this);

#endif
