#ifndef MYT_H_
#define MYT_H_

#include "State.h"

inline State *buildMYT(const char *restart, const char *reend, int flags);

inline State *basisNFA(SymbolVector *symbols)
{
    State *accept, *start;

    accept=state(symbols, NONE, NULL);
    start=state(epsilonVector(), NONE, accept);
    addState(start, accept);
    start->lchild=accept;

    accept=state(epsilonVector(), NONE, NULL);
    start->mate->lchild=accept;
    addState(start, accept);
    start->mate=accept;

    return start;
}

inline State *literalNFA(int literal, int mode)
{
    SymbolVector *symbols;
    int reChar;

    symbols=symbolVector();

    if (mode & MINUS) {
        for (reChar=1; reChar <=CHAR_MAX; ++reChar)
            if (legalChar(reChar) && reChar != literal)
                addSymbol(symbols, reChar);
    }
    
    else addSymbol(symbols, literal);

    return basisNFA(symbols);
}

inline State *classNFA(int classCode, int mode)
{
    SymbolVector *symbols;
    int reChar;

    symbols=symbolVector();

    if (mode & MINUS) {
        for (reChar=1; reChar <= CHAR_MAX; ++reChar)
            if (legalChar(reChar) && !isCharClassMember(reChar, classCode))
                addSymbol(symbols, reChar);

    } else {
        for (reChar=1; reChar <= CHAR_MAX; ++reChar)
            if (legalChar(reChar) && isCharClassMember(reChar, classCode))
                addSymbol(symbols, reChar);
    }

    return basisNFA(symbols);

}

inline State *wildcardNFA(int flags, int mode)
{
    int reChar, dotall=flags & DOTALL;
    SymbolVector *symbols;

    symbols=symbolVector();

    if (mode & MINUS) {
        for (reChar=1; reChar <= CHAR_MAX; ++reChar)
            if (legalChar(reChar) && !dotall && isspace(reChar))
                addSymbol(symbols, reChar);

    } else {
        for (reChar=1; reChar <= CHAR_MAX; ++reChar)
            if (legalChar(reChar) && (dotall || !isspace(reChar)))
                addSymbol(symbols, reChar);
    }

    return basisNFA(symbols);
}

inline State *concatNFA(State *start0, State *start1)
{
    if (!(start0 && start1)) return NULL;
    addStates(start0, start1->states);
    addState(start0, start1);

    if (!start0->mate->lchild)
        start0->mate->lchild=start1;

    else start0->mate->rchild=start1;

    start0->mate=start1->mate;
    return start0;
}

inline State *disjunctNFA(State *start0, State *start1)
{
    State *accept, *start;

    if (!(start0 && start1)) return NULL;

    accept=state(epsilonVector(), NONE, NULL);
    start=state(epsilonVector(), NONE, accept);

    addStates(start, start0->states);
    addStates(start, start1->states);
    addState(start, accept);
    addState(start, start0);
    addState(start, start1);

    start->lchild=start0;
    start->rchild=start1;

    start0->mate->lchild=accept;
    start1->mate->lchild=accept;

    start0->mate=accept;
    start1->mate=accept;

    return start;
}

inline State *closureNFA(State *start0, int greedy)
{
    State *accept, *start;

    if (!start0) return NULL;

    accept=state(epsilonVector(), NONE, NULL);
    start=state(epsilonVector(), NONE, accept);

    addStates(start, start0->states);
    addState(start, start0);
    addState(start, accept);

    if (greedy) {
        start->lchild=start0;
        start->rchild=accept; /* create the skip */

        start0->mate->lchild=start0; /* create the loop */
        start0->mate->rchild=accept;

    } else {
        start->lchild=accept; /* create the skip */
        start->rchild=start0;

        start0->mate->lchild=accept;
        start0->mate->rchild=start0; /* create the loop */
    }

    start0->mate=accept;

    return start;
}

inline State *repeatNFA(State *start0, int greedy)
{
    State *accept, *start;

    if (!start0) return NULL;

    accept=state(epsilonVector(), NONE, NULL);
    start=state(epsilonVector(), NONE, accept);

    addStates(start, start0->states);
    addState(start, start0);
    addState(start, accept);

    start->lchild=start0;

    if (greedy) {
        start0->mate->lchild=start0; /* create the loop */
        start0->mate->rchild=accept;

    } else {
        start0->mate->lchild=accept; 
        start0->mate->rchild=start0; /* create the loop */
    }

    start0->mate=accept;

    return start;
}

inline State *zeroOrOneNFA(State *start0, int greedy)
{
    State *accept, *start;

    if (!start0) return NULL;

    accept=state(epsilonVector(), NONE, NULL);
    start=state(epsilonVector(), NONE, accept);

    addStates(start, start0->states);
    addState(start, start0);
    addState(start, accept);

    start0->mate->lchild=accept;

    if (greedy) {
        start->lchild=start0;
        start->rchild=accept; /* create the skip */

    } else {
        start->lchild=accept; /* create the skip */
        start->rchild=start0;

    }

    start0->mate=accept;

    return start;
}

/**
 * Make an NFA for a non-negated set of brackets.
 */
State *posBracket(const char *brackstart, const char *brackend)
{
    SymbolVector *symbols;
    int reChar;

    symbols=symbolVector();

    if (!(brackstart && brackend)) return NULL;
    
    if (brackstart == brackend) {
        addSymbol(symbols, EPSILON);
        
    } else {
        while (brackstart != brackend) {
            if (*brackstart == '\\' && isCharClass(*++brackstart)) {
                for (reChar=1; reChar <= CHAR_MAX; ++reChar)
                    if (legalChar(reChar) && isCharClassMember(reChar, *brackstart))
                        addSymbol(symbols, reChar);

            } 
            
            else addSymbol(symbols, *brackstart);

            brackstart++;
        }
    }

    return basisNFA(symbols);
}

/**
 * Make an NFA for a negated set of brackets.
 */
State *negBracket(const char *brackstart, const char *brackend)
{
    SymbolVector *symbols;
    int reChar;

    symbols=symbolVector();

    if (!(brackstart && brackend)) return NULL;
    
    if (brackstart == brackend) {
        addSymbol(symbols, EPSILON);
        
    } else {
        symVecCompliment(symbols, symbols);
        removeSymbol(symbols, EPSILON);

        while (brackstart != brackend) {
            if (*brackstart == '\\' && isCharClass(*++brackstart)) {
                for (reChar=1; reChar <= CHAR_MAX; ++reChar)
                    if (legalChar(reChar) && isCharClassMember(reChar, *brackstart))
                        removeSymbol(symbols, reChar);

            } 
            
            else removeSymbol(symbols, *brackstart);

            brackstart++;
        }
    }

    return basisNFA(symbols);
}

/**
 * Make an NFA for a set of brackets.
 */
inline State *bracketNFA(const char *brackstart, const char *brackend, int mode)
{
    if (!(brackstart && brackend)) return NULL;
    
    if (brackstart == brackend) { /* empty brackets */
        return literalNFA(EPSILON, NONE);

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
    State *accept=state(epsilonVector(), NONE, NULL);
    State *curr, *accept0;
    int count=0;

    while (++count < maxReps) {

        curr=buildMYT(restart, reend, flags);
        accept0=start->mate;

        if (!greedy && count >= minReps)
            accept0->lchild=accept;

        start=concatNFA(start, curr);

        if (greedy && count >= minReps)
            accept0->rchild=accept;
    }

    start->mate->lchild=accept;
    addState(start, accept);
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
