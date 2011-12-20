#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "nfa.h"

int legalChar(int c) { return isgraph(c) || isspace(c); }

State *addChild(State *s, int t, State *c)
{
    if (!s || !c) return NULL;
    s->trans[t] = node(c, s->trans[t]);
    return c;
}

Node *getChild(State *s, int t)
{
    if (!s) return NULL;
    return s->trans[t];
}

void addState(NFA *nfa, State *s)
{
    addChild(nfa->start, STATE_LIST, s);
}

int isCharClass(char c)
{
    c = tolower(c);
    return c == 'w' || c == 'd';
}

int isCharClassMember(char c, char class, int mode)
{
    switch (tolower(class)) {
        case 'w':
            return mode == PLUS ? isalnum(c) : !isalnum(c);
        case 'd':
            return mode == PLUS ? isdigit(c) : !isdigit(c);
        default:
            return 0;
    }
}

void doCharClass(int class, int mode, State *prnt, State *child)
{
    int reChar;
    if (isupper(class)) {
        class=tolower(class);
        mode=-mode;
    }
    for (reChar=1; reChar <=CHAR_MAX; ++reChar) {
        if (isCharClassMember(reChar, class, mode))
            addChild(prnt, reChar, child);
    } 
}

void doLiteral(int literal, int mode, State *prnt, State *child)
{
    int reChar;
    if (mode == PLUS) addChild(prnt, literal, child);
    else
        for (reChar=1; reChar <= CHAR_MAX; ++reChar)
            if (legalChar(reChar) && reChar != literal) addChild(prnt, reChar, child);
}

void doWildcard(int mode, State *prnt, State *child, int dotAll)
{
    int reChar;
    if (mode == PLUS) {
        for (reChar=1; reChar <= CHAR_MAX; ++reChar)
            if (legalChar(reChar) && (!isspace(reChar) || dotAll)) addChild(prnt, reChar, child);
    } else {
        for (reChar=1; reChar <= CHAR_MAX; ++reChar)
            if (legalChar(reChar) && isspace(reChar) && !dotAll) addChild(prnt, reChar, child);
    }
}

const char *doBrackets(const char *currChar, int mode, State *prnt, State *child, int dotAll)
{
    int reChar;
    if (mode == MINUS) {
        for (reChar=1; reChar <= CHAR_MAX; ++reChar)
            if (legalChar(reChar)) addChild(prnt, reChar, child);
    }

    while (*currChar != ']') {
        if (*currChar == '\\') {
            currChar++;
            if (isCharClass(*currChar)) { /* character class */
                if (mode == PLUS) doCharClass(*currChar, mode, prnt, child);
                else {
                    for (reChar=1; reChar <= CHAR_MAX; ++reChar)
                        if (isCharClassMember(reChar, *currChar, PLUS))
                            prnt->trans[reChar]=popNode(prnt->trans[reChar]);
                }
            } else if (mode == PLUS) addChild(prnt, *currChar, child); /* escaped literal in plus mode */
            else prnt->trans[reChar]=popNode(prnt->trans[reChar]); /* escaped literal in minus mode */
        } else if (*currChar == WILDCARD) { /* wildcard */
            if (mode == PLUS) {
                doWildcard(PLUS, prnt, child, dotAll);
            } else {
                for (reChar=1; reChar <= CHAR_MAX; ++reChar)
                    if (legalChar(reChar) && (!isspace(reChar) || dotAll))
                        prnt->trans[reChar]=popNode(prnt->trans[reChar]);
            }
        } else { /* literal */
            if (mode == PLUS) addChild(prnt, *currChar, child);
            else prnt->trans[(int)*currChar]=popNode(prnt->trans[(int)*currChar]);
        }
        currChar++;
    }
    return currChar;
}

State *doSkip(State *prnt, State *child)
{
    return addChild(prnt, EPSILON, addChild(child, EPSILON, state(PLUS, NULL)));
}

NFA *buildNFA(const char *re, int flags)
{
    State *prev, *curr, *next, *start, *accept;
    Node *subExprStack;
    const char *currChar;
    int stateMode, dotAll;
    NFA *newNFA;

    /* malloc accepting and start states */
    accept=state(PLUS, NULL);
    start=state(PLUS, accept);

    /* malloc NFA */
    newNFA=(NFA *)malloc(sizeof(NFA));
    newNFA->start=start;
    newNFA->accept=accept;
    newNFA->flags=flags;

    /* add top level of NFA to sub-expression stack */
    subExprStack=node(start, NULL);

    /* initialize curr and prev for the main loop and add curr to the NFA */
    curr=state(PLUS, NULL);
    addChild(start, EPSILON, curr);
    addState(newNFA, curr);
    prev=start;

    dotAll=flags & DOTALL; /* get DOTALL flag */
    stateMode=PLUS; /* initialize mode */
    currChar=re; /* initialize current character */
    
    while (*currChar) {
        while (*currChar == '~') { /* negation */
            stateMode=-stateMode;
            currChar++;
        }

        if (*currChar == '\\') { /* escape and character class */
            prev=curr;
            curr=state(PLUS, NULL);
            addState(newNFA, curr);
            currChar++;

            if (isCharClass(*currChar)) { /* character class */
                doCharClass(*currChar, stateMode, prev, curr);
            } else { /* escape */
                doLiteral(*currChar, stateMode, prev, curr);
            }

        } else if (*currChar == '[') { /* bracketed character class */
            prev=curr;
            curr=state(PLUS, NULL);
            addState(newNFA, curr);
            currChar++;

            if (*currChar == '^') {
                stateMode=-stateMode;
                currChar++;
            }

            currChar=doBrackets(currChar, stateMode, prev, curr, dotAll);

        } else if (*currChar == '(') {
            prev=curr;
            curr=state(PLUS, NULL); /* startpoint for subexpression */
            next=state(stateMode, state(PLUS, NULL)); /* right paren and mate right paren */

            /* maintain path */
            addChild(prev, EPSILON, curr); 
            addChild(curr, EPSILON, next);

            subExprStack=node(curr, subExprStack); /* push pair onto sub expression stack */
            addState(newNFA, curr); /* record startpoint for subexpression */
            addState(newNFA, next); /* record left paren */
            addState(newNFA, next->mate); /* record right paren */

            /* advance prev/curr */
            prev=curr;
            curr=next;

        } else if (*currChar == ')') {
            prev=subExprStack->s; /* state before left paren */
            subExprStack=popNode(subExprStack); /* pop paren pair from stack */
            next=getChild(prev, EPSILON)->s->mate; /* get right paren */
            addChild(curr, EPSILON, next); /* connect end of branch to right paren */
            curr=next; /* continue from right paren */

        } else if (*currChar == '|') {
            prev=getChild(subExprStack->s, EPSILON)->s; /* get startpoint */
            next=prev->mate; /* get endpoint */
            if (!next) next=accept; /* if startpoint is at toplevel */
            addChild(curr, EPSILON, next); /* connect current branch to its endpoint */

            curr=state(PLUS, NULL);
            addChild(prev, EPSILON, curr);
            addState(newNFA, curr);

        } else if (*currChar == '+') {
            if (*(currChar-1) == ')' && *(currChar-2) != '\\') {
                /* loop to left paren */
                prev=getChild(prev, EPSILON)->s;
                addChild(curr, EPSILON, prev); 
            } else { /* loop to self */
                addChild(curr, *(currChar-1), curr);
            }

        } else if (*currChar == '?') {
            next=doSkip(prev, curr); /* skip from prev to curr */
            addState(newNFA, next); /* record new state */
            curr=next; /* continue from skip */

        } else if (*currChar == '*') {

            /* save current prev for the skip */
            subExprStack=node(prev, subExprStack);

            if (*(currChar-1) == ')' && *(currChar-2) != '\\') {
                /* loop to left paren */
                prev=getChild(prev, EPSILON)->s;
                addChild(curr, EPSILON, prev);
            } else { /* loop to self */
                addChild(curr, *(currChar-1), curr);
            }

            /* retrive old prev */
            prev=subExprStack->s;
            subExprStack=popNode(subExprStack);

            next=doSkip(prev, curr); /* skip from prev to curr */
            addState(newNFA, next); /* record new state */
            curr=next; /* continue from skip */

        } else if (*currChar == WILDCARD) {
            prev=curr;
            curr=state(PLUS, NULL);
            doWildcard(stateMode, prev, curr, dotAll);
        } else { /* literal */
            prev=curr;
            curr=state(PLUS, NULL);
            addState(newNFA, curr);

            doLiteral(*currChar, stateMode, prev, curr);
        }

        stateMode=PLUS;
        currChar++;
    }

    /* empty subexpression stack */
    while ((subExprStack=popNode(subExprStack)));

    /* connect last branch to accepting state */
    addChild(curr, EPSILON, accept);

    return newNFA;
}

Node *node(State *s, Node *n)
{
    Node *no = (Node *)malloc(sizeof(Node));
    no->s = s;
    no->next = n;
    return no;
}

Node *popNode(Node *n)
{
    Node *o = n;
    if (o) {
        n = n->next;
        free(o);
    }
    return n;
}

State *state(int mode, State *mate)
{
    State *s = (State *)malloc(sizeof(State));
    s->mode = mode;
    s->mate = mate;
    memset(s->trans, 0, NUM_SYMB*sizeof(Node *));
    return s;
}

