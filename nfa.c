/**
 * nfa.c
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>

#include "nfa.h"

int main(int argc, char **argv)
{
    nfa *d;
    if (argc > 2) {
        d = construct_nfa(argv[1]);
        printf("%s match %s = %d\n", argv[1], argv[2], match(d->start, argv[2], DOTALL));
        printf("%s search %s = %d\n", argv[1], argv[2], search(d->start, argv[2], DOTALL));
        printf("This nfa has %d states\n", d->n);
        free_nfa(d);
    }
    return 0;
}

state * stalloc(int c, state * c0, state * c1)
{
    state * s = (state *)malloc(sizeof(state));
    s->s = c;
    s->c[0] = c0;
    s->c[1] = c1;
    return s;
}

int match(state *s, char *str, int options)
{
    if (!s) return 0;

    if (epsilon(s))
        return match(s->c[0], str, options) || match(s->c[1], str, options);

    if (!*str) return accepting(s) ? 1 : 0;

    if (s->s == *str || (s->s == DOT && ((options & DOTALL) || !isspace(*str))))
    	return match(s->c[0], str+1, options) || match(s->c[1], str+1, options);
    
    return 0;
}

int search(state *s, char *str, int options)
{
    node *frontier, *successors;
    int start, i, k, end, match;
    state *curr;
    char *c;

    if (!s) return 0;

    successors = NULL;
    match = 0;

    for (start = 0; start < strlen(str) && !match; ++start) {

        for (end = strlen(str); end > start && !match; --end) {

            frontier = push(s, NULL);

            /* consume the substring */
            for (i = start; i < end && frontier && !match; ++i) {
                while (frontier) {
                    curr = pop(&frontier);
                    if (epsilon(curr)) {
                        for (k = 0; k < 2; ++k)
                            if (curr->c[k])
                                frontier = push(curr->c[k], frontier);
                    } else if (curr->s == str[i] || (curr->s == DOT && ((options & DOTALL) || !isspace(str[i])))) {
                        for (k = 0; k < 2; ++k)
                            if (curr->c[k])
                                successors = push(curr->c[k], successors);
                    }
                }
                frontier = successors;
                successors = NULL;
            }

            /* search the frontier for an accepting state */ 
            while (frontier) {
                curr = pop(&frontier);
                if (epsilon(curr)) {
                    for (k = 0; k < 2; ++k)
                        if (curr->c[k])
                            frontier = push(curr->c[k], frontier);
                } else if (accepting(curr)) {
                    match = 1;
                    while (frontier)
                        pop(&frontier);
                }
            }
        }
    }
    return match;
}

int epsilon(state *s) { return s->s > DOT; }

int accepting(state * s) { return s->s == ACCEPT; }

node * make_node(state * s, node * n)
{
    node *new_node = (node *)malloc(sizeof(node));
    new_node->s = s;
    new_node->next = n;
    return new_node;
}

node * push(state *s, node *n) { return make_node(s, n); }
state * pop(node **n)
{
    state *s = (*n)->s;
    node *prev = *n;
    *n = (*n)->next;
    free(prev);
    return s;
}

nfa * construct_nfa(char *re)
{
    nfa *n = (nfa *)calloc(sizeof(nfa), 1);
    node *lstack, *rstack;
    state *curr, *next, *prev;
    int brackets = 0;
    char *c = re;

#define pushstates(s) { n->states=push(s, n->states); n->n++; }

#define localroot() { \
    prev = lstack->s->c[0]; \
    while (prev->c[0] && prev->c[1]) \
        prev = prev->c[1]; \
    if (!prev->c[0]) \
        prev = prev->c[0] = stalloc(EPSILON, NULL, prev->c[0]); \
    else prev = prev->c[1] = stalloc(EPSILON, NULL, prev->c[1]); \
} \

#define pushsub(l, r) { \
    lstack = push(stalloc(EPSILON, NULL, NULL), lstack); \
    rstack = push(stalloc(r, NULL, NULL), rstack); \
    curr->c[0] = lstack->s; \
    lstack->s->c[0] = stalloc(l, NULL, NULL); \
    curr = lstack->s->c[0]; \
    prev = lstack->s; \
    pushstates(prev); \
} \

#define popsub() { \
    curr->c[0] = rstack->s; \
    rstack->s->c[0] = stalloc(EPSILON, NULL, NULL); \
    pushstates(rstack->s); \
    curr = rstack->s->c[0]; \
    prev = rstack->s; \
    if (*(c+1) == '+') { \
        rstack->s->c[1] = lstack->s->c[0]; \
        c++; \
    } \
    else if (*(c+1) == '*') { \
        lstack->s->c[1] = rstack->s->c[0]; \
        rstack->s->c[1] = lstack->s->c[0]; \
        c++; \
    } \
    pop(&lstack); \
    pop(&rstack); \
}

#define addliteral(s) { next = stalloc(s, NULL, NULL); curr->c[0] = next; prev = curr; curr = next; }

    n->start = curr = stalloc(EPSILON, NULL, NULL);
    n->accept = stalloc(ACCEPT, NULL, NULL);

    lstack = make_node(n->start, NULL);
    rstack = make_node(n->accept, NULL);
    
    pushstates(n->start);
    pushstates(n->accept);

    while (*c) {

        switch (*c) {

            case '\\':
                addliteral(*++c);
                break;

            case '[': /* start of brackets */
                pushsub(LBRACKET, RBRACKET);
                brackets = 1;
                break;

            case ']':
                brackets = 0;
                popsub();
                break;

            case '(': /* start of subexpression */
                pushsub(LPAREN, RPAREN);
                break;

            case ')': /* end of subexpression */
                popsub();
                break;

            case '|': /* alternation */
                curr->c[0] = rstack->s; /* connect this branch to its endpoint */
                localroot(); /* set prev equal to the local root for this alternation */
                curr = prev; /* ready the root to receive children */
                break;

            case '+':
                next = stalloc(EPSILON, NULL, curr);
                curr->c[0] = next;
                curr = next;
                break;

            case '*': /* kleene closure */
                next = stalloc(EPSILON, NULL, curr);
                prev->c[0] = next;
                curr->c[0] = next;
                curr = next;
                break;

            case '.': /* wild card */
                next = stalloc(DOT, NULL, NULL);
                curr->c[0] = next;
                prev = curr;
                curr = next;
                break;

            default: /* literals */
                if (brackets) {
                    localroot(); /* set prev equal to a new root for the literal */
                    next = stalloc(*c, rstack->s, NULL); /* create a state for the literal */
                    prev->c[0] = next; /* set the new state as the root's child */
                    pushstates(prev); /* record prev */
                    curr = next; /* make sure curr is prepared to continue the chain */
                } else {
                    addliteral(*c);
                }
                break;

        } /* switch */

        pushstates(curr);
        c++;
            
    } /* while */

    curr->c[0] = rstack->s;

    pop(&lstack);
    pop(&rstack);

    return n;
}

void free_nfa(nfa *n)
{
    while (n->states)
        pop(&n->states);
    free(n);
}
