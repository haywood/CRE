/**
 * nfa.c
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>

#include "nfa.h"

int main(int argc, char **argv)
{
    match_object m;
    unsigned int k;
    group *g;
    nfa *d;
    if (argc > 2) {
        d = construct_nfa(argv[1]);
        printf("%s search %s = %d\n", argv[1], argv[2], search(d, argv[2], 0, &m, DOTALL));
        printf("%d groups:\n", m.n);
        g = m.groups;
        while (g) {
            for (k = g->i[0]; k < g->i[1]; ++k)
                putchar(m.str[k]);
            putchar('\n');
            g = g->next;
        }
        printf("This nfa has %d states\n", d->n);
        free_nfa(d);
    }
    return 0;
}

state * stalloc(int c, state * c0, state * c1, int m)
{
    state * s = (state *)malloc(sizeof(state));
    s->s = c;
    s->c[0] = c0;
    s->c[1] = c1;
    s->mode = m;
    return s;
}

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

void append_result(state *s, int i, result_node *l, result_node *p, result_node *n)
{
    while (l->next)
        l = l->next;
    l->next = push_result(s, i, p, n);
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

group *make_group(int start, int end, group *n)
{
    group *g = (group *)malloc(sizeof(group));
    g->i[0] = start;
    g->i[1] = end;
    g->next = n;
    return g;
}

int matches(state *s, char c, int options)
{
    int matched = s->mode > 0 ? 1 : 0;
    if (s->s == c) return matched;
    if (s->s == DOT && ((options & DOTALL) || !isspace(c)))
        return matched;
    return !matched;
}

int search(nfa *n, char *str, unsigned int start, match_object *groups, int options)
{
    search_node *start_node, *list[2];
    unsigned int nrgroup, *rgroup;
    unsigned int i, k, end, match;
    result_node *frontier, *cres;
    state *cstate;

    if (!n) return 0;
    
    groups->groups = NULL;
    groups->str = NULL;
    groups->n = 0;
    match = 0;

    for (; start < strlen(str) && !match; ++start) {
        for (end = strlen(str); end > start && !match; --end) {

            list[0] = start_node = push_search(NULL);
            start_node->n = push_result(n->start, start, NULL, NULL);
            rgroup = NULL;
            nrgroup = 0;

            for (i = start; i < end; ++i) {

                list[1] = push_search(list[0]);
                frontier = list[0]->n;

                while (frontier) {
                    cstate = frontier->s;
                    if (epsilon(cstate)) {
                        for (k = 0; k < N_KIDS; ++k)
                            if (cstate->c[k]) { append_result(cstate->c[k], i, frontier, frontier, NULL); }
                    } else if (matches(cstate, str[i], options)) {
                            for (k = 0; k < N_KIDS; ++k)
                                if (cstate->c[k]) {
                                    if (list[1]->n) { append_result(cstate->c[k], i+1, list[1]->n, frontier, NULL); } 
                                    else { list[1]->n = push_result(cstate->c[k], i+1, frontier, NULL); }
                                }
                    } else { frontier->i = -1; }
                    frontier = frontier->next;
                } /* while */
                list[0] = list[1];
            } /* for */

            frontier = list[0]->n;
            while (frontier) {
                cstate = frontier->s;
                if (epsilon(cstate)) {
                    for (k = 0; k < N_KIDS; ++k)
                        if (cstate->c[k]) { append_result(cstate->c[k], end, frontier, frontier, NULL); }
                } else if (accepting(cstate)) {

                    match = 1;
                    groups->str = (char *)malloc((1+strlen(str))*sizeof(char));
                    groups->groups = NULL;
                    groups->n = 0;
                    memcpy(groups->str, str, (1+strlen(str))*sizeof(char));
                    cres = frontier;

                    while (cres) {
                        if (cres->s->s == RPAREN) {
                            rgroup = (unsigned int *)realloc(rgroup, (nrgroup+1)*sizeof(unsigned int));
                            rgroup[nrgroup] = cres->i;
                            nrgroup++;
                        } else if (nrgroup && cres->s->s == LPAREN) {
                            groups->groups = make_group(cres->i, rgroup[nrgroup-1], groups->groups);
                            groups->n++;

                            nrgroup--;
                            rgroup = (unsigned int *)realloc(rgroup, nrgroup*sizeof(unsigned int));
                        }
                        cres = cres->parent;
                    } /* while */
                    break;
                } /* else if */
                frontier = frontier->next;
            } /* while */

            frontier = list[0]->n;
            while (frontier) {
                while (frontier)
                    pop_result(&frontier);
                pop_search(&list[0]);
                if (list[0]) 
                    frontier = list[0]->n;
            }

            free(rgroup);

            if (n->accept->s == CASH) break; /* anchor at end */
        } /* for */
        if (n->start->s == CARET) break; /* anchor at start */
    } /* for */

    return match;
}

int epsilon(state *s) { return s->s > DOT; }

int accepting(state * s) { return s->s <= CASH; }

nfa * construct_nfa(char *re)
{
    nfa *n = (nfa *)calloc(sizeof(nfa), 1);
    node *lstack, *rstack;
    state *curr, *next, *prev;
    int brackets = 0, mode = 1;
    char *c = re;

#define pushstates(s) { n->states=push(s, n->states); n->n++; }

#define localroot() { \
    prev = lstack->s->c[0]; \
    while (prev->c[0] && prev->c[1]) \
        prev = prev->c[1]; \
    if (!prev->c[0]) \
        prev = prev->c[0] = stalloc(EPSILON, NULL, prev->c[0], mode); \
    else prev = prev->c[1] = stalloc(EPSILON, NULL, prev->c[1], mode); \
} \

#define pushsub(l, r) { \
    lstack = push(stalloc(EPSILON, NULL, NULL, mode), lstack); \
    rstack = push(stalloc(r, NULL, NULL, mode), rstack); \
    curr->c[0] = lstack->s; \
    lstack->s->c[0] = stalloc(l, NULL, NULL, mode); \
    curr = lstack->s->c[0]; \
    prev = lstack->s; \
    pushstates(prev); \
} \

#define popsub() { \
    curr->c[0] = rstack->s; \
    rstack->s->c[0] = stalloc(EPSILON, NULL, NULL, mode); \
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
    mode = 1; \
}

#define addliteral(s) { curr->c[0] = stalloc(s, NULL, NULL, mode); prev = curr; curr = curr->c[0]; }

    n->start = curr = stalloc(EPSILON, NULL, NULL, mode);
    n->accept = stalloc(ACCEPT, NULL, NULL, mode);

    lstack = make_node(n->start, NULL);
    rstack = make_node(n->accept, NULL);

    curr = n->start->c[0] = stalloc(EPSILON, NULL, NULL, mode);
    
    pushstates(n->start);
    pushstates(n->accept);
    pushstates(curr);

    while (*c) {

        switch (*c) {

            case '~':
                mode = -1;
                if (*(c+1) != '(' && *(c+1) != '[') {
                    addliteral(*++c);
                    mode = 1;
                } else addliteral(EPSILON);
                break;

            case '^':
                if (curr == n->start->c[0]) {
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
                next = stalloc(EPSILON, NULL, curr, mode);
                curr->c[0] = next;
                curr = next;
                break;

            case '*': /* kleene closure */
                next = stalloc(EPSILON, NULL, curr, mode);
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
                    next = stalloc(*c, rstack->s, NULL, mode); /* create a state for the literal */
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
