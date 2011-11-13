#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "nfa.h"

State *state(int mode, State *mate)
{
    State *s = (State *)malloc(sizeof(State));
    s->mode = mode;
    s->mate = mate;
    memset(s->trans, 0, NUM_SYMB*sizeof(Node *));
    return s;
}

Node *node(State *s, Node *n)
{
    Node *no = (Node *)malloc(sizeof(Node));
    no->s = s;
    no->next = n;
    return no;
}

Node *pop(Node *n)
{
    Node *o = n;
    if (o) {
        n = n->next;
        free(o);
    }
    return n;
}

Group *group(unsigned int b, unsigned int e, Group *n)
{
    Group *g = (Group *)malloc(sizeof(Group));
    g->i[0] = b;
    g->i[1] = e;
    g->next = n;
    return g;
}

MatchObject *matchObject(char *str, unsigned n, Group *g)
{
    MatchObject *m = (MatchObject *)malloc(sizeof(MatchObject));
    m->str = malloc((1+strlen(str))*sizeof(char));
    memcpy(m->str, str, (1+strlen(str))*sizeof(char));
    m->n = n;
    m->groups = g;
    return m;
}

void add_group(MatchObject *m, unsigned b, unsigned e)
{
    unsigned i = m->n, swapped;
    Group g[2];

    m->groups = (Group *)realloc(m->groups, ++m->n*sizeof(Group));
    m->groups[m->n-1].i[0] = b;
    m->groups[m->n-1].i[1] = e;
    do {
        swapped = 0;
        for (; i > 0; --i) {
            g[0] = m->groups[i];
            g[1] = m->groups[i-1];
            if (g[0].i[0] < g[1].i[0] || (g[0].i[0] == g[1].i[0] && g[0].i[1] > g[1].i[1])) {
                m->groups[i-1] = g[0];
                m->groups[i] = g[1];
                swapped = 1;
            }

        }
    } while (swapped);
}


State *add_child(State *s, int t, State *c)
{
    s->trans[t] = node(c, s->trans[t]);
    return c;
}

NFA * nfa(char *re, int options)
{
    NFA *n = (NFA *)malloc(sizeof(NFA));
    State *curr, *prev, *next;
    Node *delim;
    int mode;
    char *c;
    int i;

    mode = PLUS;
    n->accept = state(mode, NULL);
    n->start = state(mode, n->accept);
    delim = node(n->start, NULL);
    c = re;
    prev = curr = add_child(n->start, EPSILON, state(mode, NULL));

    while (*c) {

        while (*c == '~') { /* negation */

            mode = - mode;
            c++;
        }
    
        if (*c == '^' && c == re) { /* front anchor */

            n->start->mode = CARET;

        } else if (*c == '$' && !*(c+1)) { /* back anchor */

            n->accept->mode = CASH;

        } else if (*c == '\\') { /* escape */

            prev = curr;
            c++;
            if (mode == PLUS) {
                curr = add_child(prev, *c, state(mode, NULL));
            } else {
                curr = state(mode, NULL);
                for (i = 1; i <= CHAR_MAX; ++i)
                    if (i != *c) add_child(prev, i, curr);
            }

        } else if (*c == '[') { /* brackets */

            prev = curr;
            if (*(c+1) == '^') {
                mode = - mode;
                c++;
            }
            curr = state(PLUS, NULL);
            if (mode == MINUS) {
                for (i = 1; i <= CHAR_MAX; ++i) {
                    add_child(prev, i, curr);
                }
                mode = PLUS;
                while (*++c != ']') {
                    switch (*c) {
                        case '\\':
                            prev->trans[(int)*++c] = NULL;
                            break;
                        case '.':
                            for (i = 1; i <= CHAR_MAX; ++i)
                                if (!isspace(i)) prev->trans[i] = NULL;
                            break;
                        default:
                            prev->trans[(int)*c] = NULL;
                            break;
                    }
                }
            } else {
                while (*++c != ']') {
                    switch (*c) {
                        case '\\':
                            add_child(prev, *++c, curr);
                            break;
                        case '.':
                            for (i = 1; i <= CHAR_MAX; ++i) {
                                if (!isspace(i)) add_child(prev, i, curr);
                            }
                            break;
                        default:
                            add_child(prev, *c, curr);
                            break;
                    }
                }
            }
        } else if (*c == '(') { /* start subexpression */

            curr = add_child(curr, EPSILON, state(PLUS, NULL)); /* create new level and connect it to current */
            delim = node(curr, delim); /* store the new level in delim */
            prev = curr = add_child(curr, LPAREN, state(mode, state(PLUS, NULL))); /* add lparen and its mate rparen */
            curr = add_child(curr, EPSILON, state(PLUS, NULL)); /* entry point for alternation */

        } else if (*c == ')') { /* end subexpression */

            prev = delim->s->trans[LPAREN]->s; /* get root of level */
            curr = add_child(curr, EPSILON, prev->mate); /* connect child to delim mate */
            if (*(c+1) != '+' && *(c+1) != '*' && *(c+1) != '?')
                delim = pop(delim); /* pop current level */

        } else if (*c == '|') { /* alternation */

            if (delim->next) {
                prev = delim->s->trans[LPAREN]->s;
            } else {
                prev = delim->s;
            }
            add_child(curr, EPSILON, prev->mate);
            curr = add_child(prev, EPSILON, state(PLUS, NULL));

        } else if (*c == '+') { /* one or more */

            if (delim->next) {
                add_child(curr, LPAREN, delim->s->trans[LPAREN]->s);
                delim = pop(delim);
            } else {
                for (i = 1; i <= CHAR_MAX; ++i)
                    curr->trans[i] = prev->trans[i];
            }

        } else if (*c == '*') { /* zero or more */

            if (delim->next) {
                prev = delim->s;
                add_child(curr, LPAREN, delim->s->trans[LPAREN]->s);
                delim = pop(delim);
            } else {
                for (i = 1; i <= CHAR_MAX; ++i)
                    curr->trans[i] = prev->trans[i];
            }

            next = state(PLUS, NULL);
            add_child(prev, EPSILON, next);
            curr = add_child(curr, EPSILON, next);

        } else if (*c == '?') { /* zero or one */

            if (delim->next) {
                prev = delim->s;
                delim = pop(delim);
            }
            next = state(PLUS, NULL);
            add_child(prev, EPSILON, next);
            curr = add_child(curr, EPSILON, next);

        } else if (*c == '.') { /* wildcard */

            prev = curr;
            curr = state(PLUS, NULL);
            if (mode == PLUS) {
                for (i = 1; i <= CHAR_MAX; ++i) {
                    if (!isspace(i) || (options & DOTALL)) add_child(prev, i, curr);
                }
            } else {
                for (i = 1; i <= CHAR_MAX; ++i) {
                    if (isspace(i) && !(options & DOTALL)) add_child(prev, i, curr);
                }
            }

        } else { /* literals */

            prev = curr;
            curr = state(PLUS, NULL);
            if (mode == PLUS) {
                add_child(prev, *c, curr);
            } else if (mode == MINUS) {
                for (i = 1; i <= CHAR_MAX; ++i) {
                    if (i != *c) add_child(prev, i, curr);
                }
            }

        }

        if (mode == MINUS) mode = PLUS;

        c++;

    } /* while */

    add_child(curr, EPSILON, delim->s->mate);
    delim = pop(delim);

    delim = node(n->start, NULL);
    while (delim->s->trans[EPSILON])
        delim = node(delim->s->trans[EPSILON]->s, delim);
    if (delim->s == n->accept)
        n->empty = 1;
    while (delim->next) delim = delim->next;
    free(delim);

    puts("constructed");

    return n;
}
