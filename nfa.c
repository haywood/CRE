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
        printf("%s search %s = %d\n", argv[1], argv[2], 
                search(d->start, argv[2], 0, &m, d->start->s == CARET, d->accept->s == CASH, DOTALL));
        printf("%d groups:\n", m.n);
        g = m.groups;
        while (g) {
            for (k = g->i[0]; k < g->i[1]; ++k)
                putchar(m.str[k]);
            putchar('\n');
            g = g->next;
        }
        printf("This nfa has %d states\n", d->n);
        printf("Match empty: %d\n", d->match_empty);
        free_nfa(d);
    }
    return 0;
}

node * make_node(state * s, node * n)
{
    node *new_node = (node *)malloc(sizeof(node));
    new_node->s = s;
    new_node->next = n;
    return new_node;
}

state * stalloc(int c, state *child, int m)
{
    state * s = (state *)malloc(sizeof(state));
    s->s = c;
    s->child = child ? make_node(child, NULL) : NULL;
    s->mode = m;
    return s;
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

void append_result_to_search(state *s, int i, search_node *l, result_node *p, result_node *n)
{
    if (!l->n) l->n = push_result(s, i, p, n);
    else append_result(s, i, l->n, p, n);
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

int search(state *s, char *str, unsigned int start, match_object *groups, int matchstart, int matchend, int options)
{
    search_node *start_node, *list[2];
    unsigned int nrgroup, *rgroup;
    unsigned int i, end, match, len;
    result_node *frontier, *cres;
    state *cstate;
    node *child;

    if (!s) return 0;

    len = strlen(str);
    groups->groups = NULL;
    groups->str = NULL;
    groups->n = 0;
    match = 0;

    for (; start < len && !match; ++start) {
        for (end = len; end > start && !match; --end) {

            list[0] = start_node = push_search(NULL);
            start_node->n = push_result(s, start, NULL, NULL);
            rgroup = NULL;
            nrgroup = 0;

            for (i = start; i < end; ++i) {

                list[1] = push_search(list[0]);
                frontier = list[0]->n;

                while (frontier) {
                    cstate = frontier->s;
                    child = cstate->child;
                        
                    if (cstate->s == CARET || cstate->s == EPSILON || cstate->s == RBRACKET || cstate->s == RPAREN || cstate->s == LPAREN) {
                        while (child) {
                            append_result(child->s, i, frontier, frontier, NULL);
                            child = child->next;
                        }
                    } else if (cstate->s == LBRACKET) {
                        if (cstate->mode == PLUS) {
                            while (child) {
                                append_result(child->s, i, frontier, frontier, NULL);
                                child = child->next;
                            }
                        } else {
                            while (child && cstate && child->s != cstate->child->s->child->s) {
                                if (!matches(child->s, str[i], options)) {
                                    cstate = NULL;
                                }
                                child = child->next;
                            }
                            if (cstate) {
                                append_result_to_search(cstate->child->s->child->s, i+1, list[1], frontier, NULL);
                            }
                        }
                    } else if (matches(cstate, str[i], options)) {
                            while (child) {
                                append_result_to_search(child->s, i+1, list[1], frontier, NULL);
                                child = child->next;
                            }
                    }
                    frontier = frontier->next;

                } /* while */

                list[0] = list[1];

            } /* for */

            frontier = list[0]->n;
            while (frontier) {
                cstate = frontier->s;
                if (cstate->s == EPSILON || cstate->s == RBRACKET || cstate->s == RPAREN) {
                    child = cstate->child;
                    while (child) {
                        append_result(child->s, end, frontier, frontier, NULL);
                        child = child->next;
                    }
                } else if (accepting(cstate)) {
                    match = 1;
                    puts("matched");
                    groups->str = (char *)malloc((1+strlen(str))*sizeof(char));
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

            if (matchend) break; /* anchor at end */
        } /* for */
        if (matchstart) break; /* anchor at start */
    } /* for */

    return match;
}

int epsilon(state *s) { return s->s > DOT; }

int accepting(state * s) { return s->s == ACCEPT || s->s == CASH; }

state *add_child(state *p, state *c)
{
    node *s, *child;

    child = make_node(c, NULL);
    s = p->child;
    if (!s) {
        p->child = child;
    } else {
        while (s->next) 
            s = s->next;
        s->next = child;
    }
    return c;
}

void add_state(nfa *n, state *s)
{
    n->states = push(s, n->states);
    n->n++;
}

nfa * construct_nfa(char *re)
{
    result_node *frontier;
    nfa *n = (nfa *)calloc(sizeof(nfa), 1);
    state *curr, *next, *prev;
    node *lstack, *rstack;
    int mode = 1;
    char *c = re;

    lstack = rstack = NULL;
    lstack = push(stalloc(EPSILON, NULL, PLUS), lstack);
    rstack = push(stalloc(ACCEPT, NULL, PLUS), rstack);

    curr = prev = n->start = lstack->s;
    n->accept = rstack->s;

    add_state(n, lstack->s);
    add_state(n, rstack->s);

    while (*c) {

        switch (*c) {

            case '\\': /* escape */
                prev = curr;
                curr = add_child(curr, stalloc(*++c, NULL, lstack->s->mode));
                add_state(n, curr);
                break;

            case '~':/* negation */
                if (*(c+1) != '(' && *(c+1) != '[') {
                    prev = curr;
                    curr = add_child(curr, stalloc(*++c, NULL, -lstack->s->mode));
                    add_state(n, curr);
                }
                break;
                

            case '^': /* begin anchor */
                if (curr == n->start) n->start->s = CARET;
                break;

            case '$': /* end anchor */
                if (!*(c+1)) n->accept->s = CASH;
                break;

            case '[': /* brackets */
                if (*(c+1) == '^') {
                    mode = MINUS;
                    c++;
                }
                next = add_child(curr, stalloc(EPSILON, NULL, mode));
                add_state(n, add_child(next, stalloc(LBRACKET, NULL, mode)));
                lstack = push(next, lstack);
                rstack = push(stalloc(RBRACKET, NULL, mode), rstack);
                prev = lstack->s->child->s;
                while (*++c != ']') {
                    curr = add_child(prev, stalloc(*c, rstack->s, mode));
                    add_state(n, curr);
                }

                add_state(n, lstack->s);
                add_state(n, rstack->s);

                prev = lstack->s;
                curr = rstack->s;

                mode = PLUS;

                pop(&lstack);
                pop(&rstack);

                break;

            case '(': /* start of subexpression */
                lstack = push(add_child(curr, stalloc(EPSILON, NULL, mode)), lstack);
                rstack = push(stalloc(RPAREN, NULL, mode), rstack);
                add_state(n, lstack->s);
                add_state(n, rstack->s);

                curr = add_child(lstack->s, stalloc(LPAREN, NULL, mode));
                add_state(n, curr);
                curr = add_child(curr, stalloc(EPSILON, NULL, mode));
                add_state(n, curr);

                mode = 1;

                break;

            case ')': /* end of subexpression */
                add_child(curr, rstack->s);
                prev = lstack->s;
                curr = rstack->s;

                pop(&lstack);
                pop(&rstack);
                break;

            case '|': /* alternation */
                add_child(curr, rstack->s);
                prev = lstack->s->child->s;
                curr = add_child(lstack->s, stalloc(EPSILON, NULL, lstack->s->mode));
                add_state(n, curr);
                break;

            case '+': /* one or more */

                /* loop */
                if (!epsilon(curr))
                    next = add_child(curr, stalloc(EPSILON, curr, lstack->s->mode));
                else next = add_child(curr, stalloc(EPSILON, prev->child->s, lstack->s->mode));

                prev = curr;
                curr = next;

                add_state(n, curr);

                break;

            case '*': /* kleene closure */
                /* loop */
                if (!epsilon(curr))
                    next = add_child(curr, stalloc(EPSILON, curr, lstack->s->mode));
                else next = add_child(curr, stalloc(EPSILON, prev->child->s, lstack->s->mode));

                /* skip */

                add_child(prev, next);
                curr = next;

                add_state(n, curr);

                break;

            case '?': /* zero or one */
                /* skip */
                next = add_child(prev, stalloc(EPSILON, NULL, lstack->s->mode));
                curr = add_child(curr, next);
                add_state(n, curr);
                break;

            case '.': /* wild card */
                prev = curr;
                curr = add_child(curr, stalloc(DOT, NULL, lstack->s->mode));
                add_state(n, curr);
                break;

            default: /* literals */
                prev = curr;
                curr = add_child(curr, stalloc(*c, NULL, lstack->s->mode));
                add_state(n, curr);
                break;

        } /* switch */

        c++;

    } /* while */

    add_child(curr, rstack->s);
    pop(&lstack);
    pop(&rstack);

    assert(!lstack && !rstack);

    frontier = push_result(n->start, 0, NULL, NULL);
    n->match_empty = 0;
    while (frontier) {
        curr = frontier->s;
        if (accepting(curr)) {
            n->match_empty = 1;
            while (pop_result(&frontier));
        } else if (epsilon(curr)) {
            lstack = curr->child;
            while (lstack) {
                append_result(lstack->s, 0, frontier, NULL, NULL);
                lstack = lstack->next;
            }
        }
        pop_result(&frontier);
    }

    puts("constructed");

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
