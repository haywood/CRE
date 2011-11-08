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
    unsigned int *captures, k;
    nfa *d;
    if (argc > 2) {
        d = construct_nfa(argv[1]);
        printf("%s match %s = %d\n", argv[1], argv[2], match(d->start, argv[2], DOTALL));
        printf("%s search %s = %d\n", argv[1], argv[2], search(d, argv[2], 0, &captures, DOTALL));
        printf("%d groups:\n", captures[0]);
        for (k = 1; k <= captures[0]; ++k) {
            printf("%d %d %d\n", k, captures[k], captures[k+1]);
        }
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

int search(nfa *n, char *str, unsigned int start, unsigned int **captures, int options)
{
    search_node *start_node, *list[2];
    unsigned int ngroups, *groups, nrgroup, *rgroup;
    unsigned int i, k, end, match;
    state *cstate, *nstate;
    result_node *frontier, *cres;

    if (!n) return 0;

    match = 0;

    for (; start < strlen(str) && !match; ++start) {
        for (end = strlen(str); end > start && !match; --end) {

            list[0] = start_node = push_search(NULL);
            start_node->n = push_result(n->start, start, NULL, NULL);
            ngroups = nrgroup = 0;
            groups = NULL;
            rgroup = NULL;

            for (i = start; i < end; ++i) {

                list[1] = push_search(list[0]);
                frontier = list[0]->n;

                while (frontier) {
                    cstate = frontier->s;
                    if (epsilon(cstate)) {
                        for (k = 0; k < N_KIDS; ++k)
                            if (cstate->c[k]) {
                                nstate = cstate->c[k];
                                cstate->c[k] = (state *)malloc(sizeof(state));
                                *cstate->c[k] = *nstate;
                                append_result(cstate->c[k], i, frontier);
                            }
                    } else {
                        if (cstate->s == str[i] || (cstate->s == DOT && ((options & DOTALL) || !isspace(str[i])))) {
                            for (k = 0; k < N_KIDS; ++k)
                                if (cstate->c[k]) {
                                    nstate = cstate->c[k];
                                    cstate->c[k] = (state *)malloc(sizeof(state));
                                    *cstate->c[k] = *nstate;
                                    if (list[1]->n) {
                                        append_result(cstate->c[k], i+1, list[1]->n);
                                    } else {
                                        list[1]->n = push_result(cstate->c[k], i+1, frontier, NULL);
                                    }
                                }
                        } else { frontier->i = -1; }
                    }
                    frontier = frontier->next;
                }
                list[0] = list[1];
            }

            frontier = list[0]->n;
            while (frontier) {
                cstate = frontier->s;
                if (epsilon(cstate)) {
                    for (k = 0; k < N_KIDS; ++k)
                        if (cstate->c[k]) {
                            nstate = cstate->c[k];
                            cstate->c[k] = (state *)malloc(sizeof(state));
                            *cstate->c[k] = *nstate;
                            append_result(cstate->c[k], end, frontier);
                        }
                } else if (accepting(cstate)) {
                    match = 1;
                    cres = frontier;
                    while (cres) {
                        if (cres->s->s == RPAREN) {
                            rgroup = (unsigned int *)realloc(rgroup, (nrgroup+1)*sizeof(unsigned int));
                            rgroup[nrgroup] = cres->i;
                            nrgroup++;
                        } else if (cres->s->s == LPAREN) {
                            groups = (unsigned int *)realloc(groups, 2*(ngroups+1)*sizeof(unsigned int));
                            groups[2*ngroups-2] = cres->i;
                            groups[2*ngroups-1] = rgroup[nrgroup-1];
                            printf("%d %d\n", cres->i, rgroup[nrgroup-1]);
                            ngroups++;

                            nrgroup--;
                            rgroup = (unsigned int *)realloc(rgroup, nrgroup*sizeof(unsigned int));
                        }
                        cres = cres->parent;
                    }
                    *captures = (unsigned int *)malloc((2*ngroups + 1)*sizeof(unsigned int));
                    for (k = 1; k < 2*ngroups+1; ++k) { (*captures)[k] = groups[k-1]; }
                    (*captures)[0] = ngroups;
                    break;
                }
                frontier = frontier->next;
            }

            frontier = list[1]->n;
            while (frontier) {
                while (frontier)
                    pop_result(&frontier);
                pop_search(&list[1]);
                if (list[1]) 
                    frontier = list[1]->n;
            }

            free(rgroup);
            free(groups);

            if (n->accept->s == CASH) break;
        }
        if (n->start->s == CARET) break;
    }

    return match;
}

int epsilon(state *s) { return s->s > DOT; }

int accepting(state * s) { return s->s == ACCEPT || s->s == CASH; }

node * make_node(state * s, node * n)
{
    node *new_node = (node *)malloc(sizeof(node));
    new_node->s = s;
    new_node->next = n;
    return new_node;
}

node * push(state *s, node *n) { 
    if (!s) return n;
    return make_node(s, n); 
}

state * pop(node **n)
{
    node *prev;
    state *s;

    if (!*n) return NULL;

    s = (*n)->s;
    prev = *n;
    *n = (*n)->next;
    free(prev);
    return s;
}

void append_result(state *s, int i, result_node *n)
{
    while (n->next)
        n = n->next;
    n->next = push_result(s, i, n, NULL);
}

search_node *push_search(search_node *n)
{
    search_node *new_node = (search_node *)calloc(sizeof(search_node), 1);
    new_node->next = n;
    return new_node;
}

result_node *pop_search(search_node **n)
{
    search_node *prev;
    result_node *p;

    if (!*n) return NULL;

    prev = *n;
    p = (*n)->n;
    *n = (*n)->next;
    free(prev);
    return p;
}

result_node *push_result(state *s, int i, result_node *p, result_node *n)
{
    result_node *r = (result_node *)malloc(sizeof(result_node));
    r->s = s;
    r->i = i;
    r->parent = p;
    r->next = n;
    return r;
}

state *pop_result(result_node **n)
{
    result_node *prev;
    state *s;

    if (!*n) return NULL;

    prev = *n;
    s = (*n)->s;
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

#define addliteral(s) { curr->c[0] = stalloc(s, NULL, NULL); prev = curr; curr = curr->c[0]; }

    n->start = curr = stalloc(EPSILON, NULL, NULL);
    n->accept = stalloc(ACCEPT, NULL, NULL);

    lstack = make_node(n->start, NULL);
    rstack = make_node(n->accept, NULL);
    
    pushstates(n->start);
    pushstates(n->accept);

    while (*c) {

        switch (*c) {

            case '^':
                if (curr == n->start) {
                    addliteral(EPSILON);
                    n->start->s = CARET;
                }
                break;

            case '$':
                if (!*(c+1)) {
                    addliteral(EPSILON);
                    n->accept->s = CASH;
                }
                break;

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
                addliteral(DOT);
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
    while (n->states) {
        free(n->states->s);
        pop(&n->states);
    }
    free(n);
}
