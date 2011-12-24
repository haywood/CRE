#ifndef MYT_H_
#define MYT_H_

#include "nfa.h"

inline State *buildMYT(const char *restart, const char *reend, int flags);

inline State *literalNFA(int literal, int mode)
{
    State *accept, *start;
    int reChar;

    accept=state(NONE, NULL);
    start=state(mode, accept);
    addChild(start, STATE_LIST, accept);

    if (mode & MINUS) {
        for (reChar=1; reChar <=CHAR_MAX; ++reChar)
            if (legalChar(reChar) && reChar != literal)
                addChild(start, reChar, accept);
    } else addChild(start, literal, accept);

    return start;
}

inline State *classNFA(int classCode, int mode)
{
    State *accept, *start;
    int reChar;

    accept=state(NONE, NULL);
    start=state(mode, accept);
    addChild(start, STATE_LIST, accept);

    if (mode & MINUS) {
        for (reChar=1; reChar <= CHAR_MAX; ++reChar)
            if (legalChar(reChar) && !isCharClassMember(reChar, classCode))
                addChild(start, reChar, accept);

    } else {
        for (reChar=1; reChar <= CHAR_MAX; ++reChar)
            if (legalChar(reChar) && isCharClassMember(reChar, classCode))
                addChild(start, reChar, accept);
    }

    return start;

}

inline State *wildcardNFA(int flags, int mode)
{
    int reChar, dotall=flags & DOTALL, negate=mode & MINUS;
    State *accept, *start;

    accept=state(NONE, NULL);
    start=state(mode, accept);
    addChild(start, STATE_LIST, accept);

    if (negate) {
        for (reChar=1; reChar <= CHAR_MAX; ++reChar)
            if (legalChar(reChar) && !dotall && isspace(reChar))
                addChild(start, reChar, accept);

    } else {
        for (reChar=1; reChar <= CHAR_MAX; ++reChar)
            if (legalChar(reChar) && (dotall || !isspace(reChar)))
                addChild(start, reChar, accept);
    }

    return start;
}

inline State *concatNFA(State *start0, State *start1)
{
    if (!(start0 && start1)) return NULL;
    appendChildren(start0, STATE_LIST, start1);
    addChild(start0->mate, EPSILON, start1);
    start0->mate=start1->mate;
    return start0;
}

inline State *disjunctNFA(State *start0, State *start1)
{
    State *accept, *start;

    if (!(start0 && start1)) return NULL;

    accept=state(NONE, NULL);
    start=state(NONE, accept);

    appendChildren(start, STATE_LIST, start0);
    appendChildren(start, STATE_LIST, start1);
    addChild(start, STATE_LIST, accept);

    addChild(start, EPSILON, start0);
    addChild(start, EPSILON, start1);

    addChild(start0->mate, EPSILON, accept);
    addChild(start1->mate, EPSILON , accept);

    start0->mate=accept;
    start1->mate=accept;

    return start;
}

inline State *closureNFA(State *start0, int greedy)
{
    State *accept, *start;

    if (!start0) return NULL;

    accept=state(NONE, NULL);
    start=state(NONE, accept);

    appendChildren(start, STATE_LIST, start0);
    addChild(start, STATE_LIST, start0);

    if (greedy) {
        addChild(start, EPSILON, start0);
        addChild(start, EPSILON, accept); /* create the skip */

        addChild(start0->mate, EPSILON, start0); /* create the loop */
        addChild(start0->mate, EPSILON, accept);

    } else {
        addChild(start, EPSILON, accept); /* create the skip */
        addChild(start, EPSILON, start0);

        addChild(start0->mate, EPSILON, accept);
        addChild(start0->mate, EPSILON, start0); /* create the loop */
    }

    start0->mate=accept;

    return start;
}

inline State *repeatNFA(State *start0, int greedy)
{
    State *accept, *start;

    if (!start0) return NULL;

    accept=state(NONE, NULL);
    start=state(NONE, accept);

    appendChildren(start, STATE_LIST, start0);
    addChild(start, STATE_LIST, start0);

    addChild(start, EPSILON, start0);

    if (greedy) {
        addChild(start0->mate, EPSILON, start0); /* create the loop */
        addChild(start0->mate, EPSILON, accept);

    } else {
        addChild(start0->mate, EPSILON, accept);
        addChild(start0->mate, EPSILON, start0); /* create the loop */
    }

    start0->mate=accept;

    return start;
}

inline State *zeroOrOneNFA(State *start0, int greedy)
{
    State *accept, *start;

    if (!start0) return NULL;

    accept=state(NONE, NULL);
    start=state(NONE, accept);

    appendChildren(start, STATE_LIST, start0);
    addChild(start, STATE_LIST, start0);
    addChild(start0->mate, EPSILON, accept);

    if (greedy) {
        addChild(start, EPSILON, start0);
        addChild(start, EPSILON, accept); /* create the skip */
    
    } else {
        addChild(start, EPSILON, accept); /* create the skip */
        addChild(start, EPSILON, start0);
    }

    return start;
}

/**
 * Make an NFA for a non-negated set of brackets.
 */
State *posBracket(const char *brackstart, const char *brackend)
{
    State *accept, *start;
    int reChar;

    if (!(brackstart && brackend)) return NULL;
    
    accept=state(NONE, NULL);
    start=state(NONE, accept);

    if (brackstart == brackend) {
        addChild(start, EPSILON, accept);
    } else {
        while (brackstart != brackend) {
            if (*brackstart == '\\' && isCharClass(*++brackstart)) {
                for (reChar=1; reChar <= CHAR_MAX; ++reChar)
                    if (legalChar(reChar) && isCharClassMember(reChar, *brackstart))
                        addChild(start, reChar, accept);

            } else addChild(start, *brackstart, accept);
            brackstart++;
        }
    }

    return start;
}

/**
 * Make an NFA for a negated set of brackets.
 */
State *negBracket(const char *brackstart, const char *brackend)
{
    State *accept, *start;
    int reChar;

    if (!(brackstart && brackend)) return NULL;
    
    accept=state(NONE, NULL);
    start=state(NONE, accept);

    if (brackstart == brackend) {
        addChild(start, EPSILON, accept);
    } else {
        for (reChar=1; reChar <= CHAR_MAX; ++reChar)
            if (legalChar(reChar)) addChild(start, reChar, accept);

        while (brackstart != brackend) {
            if (*brackstart == '\\' && isCharClass(*++brackstart)) {
                for (reChar=1; reChar <= CHAR_MAX; ++reChar)
                    if (legalChar(reChar) && isCharClassMember(reChar, *brackstart))
                        start->trans[reChar]=popNode(start->trans[reChar]);

            } else start->trans[(int)*brackstart]=popNode(start->trans[(int)*brackstart]);
            brackstart++;
        }
    }

    return start;
}

/**
 * Make an NFA for a set of brackets.
 */
inline State *bracketNFA(const char *brackstart, const char *brackend, int mode)
{
    State *accept, *start;

    if (!(brackstart && brackend)) return NULL;
    
    if (brackstart == brackend) { /* empty brackets */
        accept=state(NONE, NULL);
        start=state(NONE, accept);
        addChild(start, EPSILON, accept);
        return start;

    } else {
        if (*brackstart == '^' && brackstart+1 != brackend) {
            mode^=MINUS;
            brackstart++;
        }

        if (mode & MINUS) return negBracket(brackstart, brackend);
        return posBracket(brackstart, brackend);
    }
}

/**
 * Concatenate start with itself maxReps - 1 times so that the resulting NFA matches repetitions of minReps - maxReps instances of the pattern recognized by start.
 */
inline State *fixedRepeatNFA(State *start, const char *restart, const char *reend, int minReps, int maxReps, int greedy, int flags)
{
    State *accept=state(NONE, NULL), *curr, *accept0;
    int count=0;

    while (++count < maxReps) {

        curr=buildMYT(restart, reend, flags);
        accept0=start->mate;

        if (!greedy && count >= minReps)
            addChild(accept0, EPSILON, accept);

        start=concatNFA(start, curr);

        if (greedy && count >= minReps)
            addChild(accept0, EPSILON, accept);
    }

    addChild(start->mate, EPSILON, accept);
    addChild(start, STATE_LIST, accept);
    start->mate=accept;

    return start;
}

/**
 * Build an NFA from the regular expression between restart (inclusive) and reend (exclusive).
 * The regular expression must be valid. This validity is currently unchecked.
 * flags is DOTALL or 0.
 */
inline State *buildMYT(const char *restart, const char *reend, int flags)
{
    int stateMode, paren, minReps, maxReps, greedy;
    const char *tokstart, *tokend, *tokmid;
    State *start, *curr;

    if (!restart || !reend)
        return NULL;

    stateMode=NONE;
    start=literalNFA(EPSILON, stateMode);

    /* empty string */
    if (restart == reend)
        return start;

    tokstart=tokend=restart;
    curr=start;

    while (tokstart != reend) {

        while (*tokend == '~') { /* negation */
            stateMode^=MINUS;
            tokend++;
        }

        /* advance tokmid and tokend */
        tokmid=tokend++; 

        if (*tokmid == '|') { /* alternation */
            return disjunctNFA(start, buildMYT(tokmid+1, reend, flags));

        } else if (*tokmid == '[') {
            while (*tokend++ != ']') {
                if (*tokend == '\\')
                    tokend++;
            }

            curr=bracketNFA(tokmid+1, tokend-1, stateMode);

        } else if (*tokmid == '(') { /* parenthetical */
            paren=1;
            while (paren) { /* find end of this parenthetical */
                if (*tokend == '(') paren++;
                else if (*tokend == ')') paren--;
                tokend++;
            }

            /* recurse on subexpressions */
            curr=buildMYT(tokmid+1, tokend-1, flags);
            curr->mode|=stateMode|LPAREN;
            curr->mate->mode|=RPAREN;

        } else if (*tokmid == WILDCARD) { /* wildcard operator */
            curr=wildcardNFA(flags, stateMode);

        } else if (*tokmid == '\\') { /* escaped literals and character classes */
        
            if (isCharClass(*(tokmid+1))) { /* character class */
                curr=classNFA(*(tokmid+1), stateMode);
            } 

            else curr=literalNFA(*(tokmid+1), stateMode); /* literal */

            tokend++;

        } else { /* literals */
            curr=literalNFA(*tokmid, stateMode);
        }

        /* quantifiers and modifiers */
        if (tokend != reend) { /* make sure not to go out of the search area of the string */
            if (*tokend == '{') { /* fixed number repetition */
                maxReps=minReps=atoi(tokend+1);
                tokmid=tokend;
                while (*++tokend != '}') {
                    if (*tokend == ',') maxReps=atoi(tokend+1);
                }
                greedy=*++tokend!='?';
                if (!greedy) tokend++;
                curr=fixedRepeatNFA(curr, tokstart, tokmid, minReps, maxReps, greedy, flags);
            }

            if (*tokend == '*') { /* closure */
                greedy=*++tokend!='?';
                if (!greedy) tokend++;
                curr=closureNFA(curr, greedy);

            } else if (*tokend == '+') { /* repetition */
                greedy=*++tokend!='?';
                if (!greedy) tokend++;
                curr=repeatNFA(curr, greedy);

            } else if (*tokend == '?') { /* zero or one */
                greedy=*++tokend!='?';
                if (!greedy) tokend++;
                curr=zeroOrOneNFA(curr, greedy);
            }
        }

        concatNFA(start, curr); /* concatenate NFA so far with current token */
        tokstart=tokend; /* advance token start pointer */
        stateMode=NONE; /* reset mode */
    }

    return start;
}

#endif /* MYT_H_ */
