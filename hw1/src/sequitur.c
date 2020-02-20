#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "const.h"
#include "sequitur.h"
#include "debug.h"

/*
 * This file implements the core of the Sequitur algorithm.
 * It is based on the description and reference implementation at www.sequitur.info.
 * I found the JavaScript reference implementation to be the least verbose and most
 * helpful of the implementations provided there.
 * Most of the comments below were added by me in the course of understanding and
 * implementing the algorithm.  I left them in because a lot of the subtleties of
 * the implementation are not explained on the website.
 *
 * E. Stark <stark@cs.stonybrook.edu>
 * January 19, 2020
 */

/**
 * Link two symbols together, handling any digram deletions that result from breaking
 * the existing link from this to this->next.
 *
 * @param this  The symbol that is to be disconnected from any existing next symbol
 * and joined to a new symbol.
 * @param next  The symbol that is to be linked as the new next neighbor of this.
 */
static void join_symbols(SYMBOL *this, SYMBOL *next) {
    debug("Join %s%lu%s and %s%lu%s",
	  IS_RULE_HEAD(this) ? "[" : "<", SYMBOL_INDEX(this), IS_RULE_HEAD(this) ? "]" : ">",
	  IS_RULE_HEAD(next) ? "[" : "<", SYMBOL_INDEX(next), IS_RULE_HEAD(next) ? "]" : ">");
    if(this->next) {
	// We will be assigning to this->next, which will destroy any digram
	// that starts at this.  So that is what we have to delete from the table.
	digram_delete(this);

	// We have to have special-case treatment of triples, which are occurrences
	// of the same three symbols in a row.  These violations of "no repeated digrams"
	// are allowed because creating a rule to replace two of the three symbols
	// would result in a rule that is only used once, thus violating the "rule utility"
	// constraint.  However, the digram hash table will only be able to record
	// one of the two overlapping digrams.  In the present situation case,
	// the deletion we have just performed has deleted the only instance of this
	// digram in the hash table, but there will still be one left once we join
	// the remaining nodes.  So we need to detect this and put the digram back
	// into the table.
	//
	// There are two cases (the ^ shows which is getting deleted):
	//    abbbc  ==> abbc   (handle this first)
	//      ^
	//    abbbc  ==> abbc   (then check for this one)
    //     ^
	if(next->prev && next->next &&
	   next->value == next->prev->value && next->value == next->next->value)
	    digram_put(next);
	if(this->prev && this->next &&
	   this->value == this->prev->value && this->value == this->next->value)
	    digram_put(this);
    }
    this->next = next;
    next->prev = this;
}

/**
 * Insert a new symbol after a specified symbol, handling any digram deletions
 * that result.
 *
 * @param this  The symbol after which the new symbol is to be inserted.
 * @param next  The symbol that is to be inserted.
 */
void insert_after(SYMBOL *this, SYMBOL *next) {
    debug("Insert symbol <%lu> after %s%lu%s", SYMBOL_INDEX(next),
	  IS_RULE_HEAD(this) ? "[" : "<", SYMBOL_INDEX(this), IS_RULE_HEAD(this) ? "]" : ">");
    join_symbols(next, this->next);
    join_symbols(this, next);
}

/**
 * Delete a symbol from a rule body, handling the deletion of any digrams that are
 * thereby destroyed.
 *
 * @param this  The symbol to be deleted.
 */
static void delete_symbol(SYMBOL *this) {
    debug("Delete symbol <%lu> (value=%d)", SYMBOL_INDEX(this), this->value);
    if(IS_RULE_HEAD(this)) {
	fprintf(stderr, "Attempting to delete a rule sentinel!\n");
	abort();
    }
    // Splice the symbol out, deleting the digram headed by the neighbor to the left.
    join_symbols(this->prev, this->next);

    // Delete the digram formed by the deleted node and its neighbor to the right.
    digram_delete(this);

    // If the deleted node is a nonterminal, decrement the reference count of
    // the associated rule.
    if(IS_NONTERMINAL(this))
	unref_rule(this->rule);

    // Recycle the deleted symbol for re-use.
    recycle_symbol(this);
}

/**
 * Expand the one remaining instance of an underutilized rule and delete the rule.
 *
 * @param this  The unique nonterminal symbol that refers to the rule to be deleted.
 */
static void expand_instance(SYMBOL *this) {
    SYMBOL *rule = this->rule;
    debug("Expand last instance of underutilized rule [%lu] for %d",
	   SYMBOL_INDEX(rule), rule->value);
    if(rule->refcnt != 1) {
	fprintf(stderr, "Attempting to delete a rule with multiple references!\n");
	abort();
    }
    SYMBOL *left = this->prev;
    SYMBOL *right = this->next;
    SYMBOL *first = rule->next;
    SYMBOL *last = rule->prev;

    // We are destroying any digram that starts at this symbol,
    // so we have to delete that from the table.
    digram_delete(this);

    // Splice the body of the rule in place of the nonterminal symbol.
    join_symbols(left, first);
    join_symbols(last, right);
    rule->next = rule->prev = NULL;  // Avoid mysterious problems.

    // Recycle the nonterminal symbol, which is no longer used.
    recycle_symbol(this);

    // Delete the now-unused rule.
    unref_rule(rule);
    delete_rule(rule);
}

/**
 * Replace a digram by a nonterminal.
 *
 * @param this  The first symbol of the digram to be replaced.
 * @param rule  The rule whose body matches the digram to be replaced and whose
 * head should replace the existing digram.
 */
static void replace_digram(SYMBOL *this, SYMBOL *rule) {
    debug("Replace digram <%lu> using rule [%lu] for %d",
	  SYMBOL_INDEX(this), SYMBOL_INDEX(rule), rule->value);
    SYMBOL *prev = this->prev;

    // Delete the two nodes of the digram headed by "this", handling the removal
    // of any digrams that are thereby destroyed.
    delete_symbol(prev->next);
    delete_symbol(prev->next);

    // Create a new nonterminal symbol that refers to the head of the rule
    // and insert it in place of the original digram.
    SYMBOL *new = new_symbol(rule->value, rule);
    insert_after(prev, new);

    // It might be the case that the insertion has created a second instance
    // of an already existing digram, so we have to check for that and take
    // appropriate action to restore the "no repeated digrams" constraint.
    // There are two digrams that might need to be checked, in general.
    // We first check and handle the digram that starts to the left, at "prev".
    // If in the process of handling that digram, it happens to get replaced,
    // then the replacement will have deleted the nonterminal that we just inserted
    // (and recursively handled any other digrams that might have gotten created),
    // so there is no need to do anything else at this point.
    // On the other hand, if that digram did not happen to get replaced, then we also
    // have to check the digram starting at prev->next, which is still headed by the
    // nonterminal we just inserted.
    if(!check_digram(prev)) {
	check_digram(prev->next);
    }
}

/**
 * This is the core function of the algorithm: handle the case in which a just-created
 * digram matches a previously existing one.
 *
 * @param this  The just-created digram.
 * @param match  The previously existing matching digram.
 */
static void process_match(SYMBOL *this, SYMBOL *match) {
    debug("Process matching digrams <%lu> and <%lu>",
	  SYMBOL_INDEX(this), SYMBOL_INDEX(match));
    SYMBOL *rule = NULL;

    if(IS_RULE_HEAD(match->prev) && IS_RULE_HEAD(match->next->next)) {
	// If the digram headed by match constitutes the entire right-hand side
	// of a rule, then we don't create any new rule.  Instead we use the
	// existing rule to replace_digram for the newly inserted digram.
	rule = match->prev->rule;
	replace_digram(this, match->prev->rule);
    } else {
	// Otherwise, we create a new rule.
	// Note that only one digram is created by this rule, and the insert_after
	// calls will only delete digrams from the hash table, but do not insert any.
	// In fact, no digrams will be deleted during the construction of
	// the new rule because the calls are being made in such a way that we are
	// never overwriting any pointers that were previously non-NULL.
	rule = new_rule(next_nonterminal_value++);
	add_rule(rule);
	insert_after(rule->prev, new_symbol(this->value, this->rule));
	insert_after(rule->prev, new_symbol(this->next->value, this->next->rule));

	// Now, replace the two existing instances of the right-hand side of the
	// rule by nonterminals that refer to the rule.
	// Note that these will potentially cause the destruction of digrams,
	// leading to their deletion from the hash table.
	// They will potentially also cause the creation of digrams, due to the
	// insertion of the nonterminal symbol.
	// However, since the nonterminal symbol is a freshly created one that
	// did not exist before, these replacements cannot result in the creation
	// of digrams that duplicate already existing ones.
	replace_digram(match, rule);
	replace_digram(this, rule);

	// Insert the right-hand side of the new rule into the digram table.
	// Note that no other rules that might have been created as a result of the
	// two substitutions above could have the same right-hand side as the rule
	// we are about to insert here, because, the right-hand sides of any of these
	// other rules must contain the new nonterminal that is at the head of the
	// current rule but not in the body of the current rule.
	digram_put(rule->next);
    }

    // We have now restored the "no repeated digram" property, but it might be that
    // deletions that occurred during the above substitutions have left us with a rule
    // that is used only once, which would violate the "rule utility" constraint.
    // In that case, we want to expand the one remaining reference to the underutilized
    // rule and delete it.
    //
    // The new rule we just created above has two symbols on the right-hand side.
    // Either of these symbols could be a nonterminal for which instances could have ended
    // up getting deleted by the substitution of the head of the new rule for occurrences
    // of its body.  However, it is only necessary to check the first symbol here,
    // because instances of the second symbol could only have gotten deleted as a result
    // of the recursive creation of a new rule with that symbol as the first symbol
    // of its body.  In that case, the underutilization check will already have been
    // performed recursively for that symbol and there is no need to do it here.
    // This is probably the most subtle point in the entire algorithm, which requires
    // substantial head-scratching to understand.

    SYMBOL *tocheck = rule->next->rule;  // The first symbol of the just-added rule.
    if(tocheck) {
	debug("Checking reference count for rule [%lu] => %d",
	      SYMBOL_INDEX(tocheck), tocheck->refcnt);
	if(tocheck->refcnt < 2) {
	    if(tocheck->refcnt == 0) {
		// There is at least one reference in the just-added rule.
		fprintf(stderr, "Reference count should not be zero!\n");
		abort();
	    }
	    expand_instance(rule->next);
	}
    }
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
    debug("Check digram <%lu> for a match", SYMBOL_INDEX(this));

    // If the "digram" is actually a single symbol at the beginning or
    // end of a rule, then there is no need to do anything.
    if(IS_RULE_HEAD(this) || IS_RULE_HEAD(this->next))
	return 0;

    // Otherwise, look up the digram in the digram table, to see if there is
    // a matching instance.
    SYMBOL *match = digram_get(this->value, this->next->value);
    if(match == NULL) {
        // The digram did not previously exist -- insert it now.
	digram_put(this);
	return 0;
    }

    // If the existing digram overlaps the one we are checking, then what we have
    // is a triple, like aaa.  In this case, we do not replace it because the resulting
    // rule would only be used once.
    if(match->next == this) {
	return 0;
    } else {
	process_match(this, match);
	return 1;
    }
}
