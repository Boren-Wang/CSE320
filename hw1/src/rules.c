#include "const.h"
#include "sequitur.h"

/*
 * Rule management.
 *
 * Conceptually, a rule has the form:
 *
 *    H ==> B1 B2 ... Bn
 *
 * where H is a nonterminal symbol, called the "head" of the rule,
 * and B1 B2 ... Bn is a list of symbols called the "body" of the rule.
 * The symbols in the body can be either terminal symbols or nonterminal symbols.
 * The idea of a rule is that it specifies a substitution that can be
 * performed, in which an instance of the head of the rule is expanded
 * by replacing it by an instance of the body of the rule.
 *
 * Here we don't introduce a separate structure for rules, but instead use
 * the existing SYMBOL structure to represent them.  A rule is represented
 * by a circular, doubly linked list of SYMBOL structures (representing the
 * body of the list), with an additional SYMBOL structure (representing the
 * head of the list) used as a "sentinel" that connects between the first
 * symbol in the body of the list and the last.
 *
 *    H <-> B1 <-> B2 <-> ... <-> Bn
 *    ^                           ^
 *    |                           |
 *    +---------------------------+
 *
 * The "next" and "prev" fields of the SYMBOL structure are used to create the
 * doubly linked list.  The sentinel node H is identifiable by the fact that it
 * has a non-NULL "rule" field that points back to itself.  Nonterminal nodes
 * in the body of the list may also have non-NULL pointers in their rule field,
 * but these will point to the heads of other rules, not back to the symbols
 * themselves.
 *
 * Rules are also maintained in a list of all rules, which is also a circular,
 * doubly linked list, but it uses the "nextr" and "prevr" fields in the SYMBOL
 * structure rather than the "next" and "prev" fields that are used within a rule.
 * The list is accessed via the "main_rule" variable, which points to the
 * head of a rule.  The heads of other rules in the list are accessed by following
 * the nextr and prevr pointers starting from the head of the main rule.
 * The main rule is not used as a special sentinel; it is simply an ordinary rule
 * like the others in the list.  When traversing the list forward and backward,
 * one can use the main_rule variable to identify when the beginning or end of
 * the list has been reached.
 */

/**
 * Initializes the rules by setting main_rule to NULL and clearing the rule_map.
 */
void init_rules(void) {
    // To be implemented.
}

/**
 * Create a new rule, with a head having a specified value.
 *
 * @param v  The value to be used for the head.  As the head of a rule represents
 * a nonterminal symbol, the specified value must be in the range of values appropriate
 * for such symbols.
 * @return  A pointer to the head of the newly created rule.  The "value" field of the
 * returned structure is initialized to the specified value.  The "rule" field is
 * initialized to point back to the structure itself.  In addition, then "next" and "prev"
 * fields are also initialized to point back to the structure itself; representing an
 * empty rule body.
 *
 * Note that the actual insertion and deletion of symbols in the body of a list is
 * the responsiblity of the client of this module.
 */
SYMBOL *new_rule(int v) {
    // To be implemented.
    return NULL;
}

/**
 * Add a rule to the end of the list of all rules.
 *
 * @param  The rule to be added.
 *
 * If main_rule is NULL, then the specified rule becomes the main rule.
 * In this case, its "nextr" and "prevr" fields are initialized to point
 * back to the rule itself, thereby creating an empty, doubly linked circular
 * list. If main_rule is not-NULL, then the rule is inserted at the end of
 * the list; i.e. between main_rule->prevr and main_rule.
 */
void add_rule(SYMBOL *rule) {
    // To be implemented.
}

/**
 * Delete a rule from the list of all rules.
 *
 * @param rule  The rule to be deleted.
 *
 * The specified rule is removed from the circular, doubly linked list of
 * all rules that is headed by main_rule.  If the reference count of the
 * rule head is zero, then the rule head is recycled, otherwise no recycling
 * is done.  Any symbols in the body of the rule are never recycled;
 * the disposition of those symbols is the responsibility of the caller.
 */
void delete_rule(SYMBOL *rule) {
    // To be implemented.
}

/**
 * Increase the reference count on a rule by one.
 *
 * @param rule  The rule whose reference count is to be increased.
 * @return  The same rule that was passed as argument.
 */
SYMBOL *ref_rule(SYMBOL *rule) {
    // To be implemented.
    return NULL;
}

/**
 * Decrease the reference count on a rule by one.
 * If the reference count would become negative, this function issues a message
 * to stderr and aborts.
 *
 * @param rule  The rule whose reference count is to be increased.
 *
 */
void unref_rule(SYMBOL *rule) {
    // To be implemented.
}
