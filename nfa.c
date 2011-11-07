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
        printf("%s ~= %s = %d\n", argv[1], argv[2], accepts(d->start, argv[2], DOTALL));
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

int accepts(state *s, char *str, int options)
{
    if (!s) return 0;

    if (epsilon(s))
        return accepts(s->c[0], str, options) || accepts(s->c[1], str, options);

    if (!*str) return accepting(s) ? 1 : 0;

    if (s->s == *str)
        return accepts(s->c[0], str+1, options) || accepts(s->c[1], str+1, options);
    
    if (s->s == DOT && ((options & DOTALL) || !isspace(*str)))
    	return accepts(s->c[0], str+1, options) || accepts(s->c[1], str+1, options);
    
    return 0;
}

int search_accepts(state *s, char *str, int options)
{
    node *clist, *olist;
    state *curr;
    int start, i, k, end;

    if (!s) return 0;

    clist = push(s->start, NULL);
    end = strlen(str);

    while (1) {
        curr = pop(clist);
        if (start == end) {
            while (clist)
                pop(clist);
            while (olist)
                pop(olist);
            return 0;
        }
        if (s->s == str[i]) {
            
        }
        if (s->s == DOT && ((options & DOTALL) || !isspace(str[i]))) {
            
        }
        for (k = 0; k < 2; ++k)
            if (curr->c[k])
                olist = push(curr->c[k], olist);
    }
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
    prev = lstack->s; \
    while (prev->c[0] && prev->c[1]) \
        prev = prev->c[1]; \
    if (!prev->c[0]) \
        prev = prev->c[0] = stalloc(EPSILON, NULL, prev->c[0]); \
    else prev = prev->c[1] = stalloc(EPSILON, NULL, prev->c[1]); \
} \

#define pushsub(l, r) { \
    lstack = push(stalloc(l, NULL, NULL), lstack); \
    rstack = push(stalloc(r, NULL, NULL), rstack); \
    curr->c[0] = lstack->s; \
    prev = curr; \
    curr = lstack->s; \
} \

#define popsub() { \
    curr->c[0] = rstack->s; \
    rstack->s->c[0] = stalloc(EPSILON, NULL, NULL); \
    pushstates(rstack->s); \
    curr = rstack->s->c[0]; \
    prev = rstack->s; \
    if (*(c+1) == '+') { \
        rstack->s->c[1] = lstack->s; \
        c++; \
    } \
    else if (*(c+1) == '*') { \
        lstack->s->c[0] = stalloc(EPSILON, lstack->s->c[0], lstack->s->c[1]); \
        rstack->s->c[1] = lstack->s->c[0]; \
        pushstates(lstack->s->c[0]); \
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
