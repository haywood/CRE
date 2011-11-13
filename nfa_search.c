#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "nfa.h"

typedef struct _SNode {
    State *s;
    unsigned i;
    struct _SNode *next;
} SNode;

SNode *snode(State *s, unsigned i, SNode *next)
{
    SNode *n = (SNode *)malloc(sizeof(SNode));
    n->s = s;
    n->i = i;
    n->next = next;
    return n;
}

SNode *append(SNode *l, unsigned i, Node *n) 
{
    SNode *tail = l;
    while (tail && tail->next) {
        tail = tail->next;
    }
    while (n) {
        if (tail) {
            tail = tail->next = snode(n->s, i, NULL);
        }
        else l = tail = snode(n->s, i, NULL);
        n = n->next;
    }
    return l;
}

SNode *search_rec(State *s, State *a, char *str, unsigned int start, unsigned int finish)
{
    SNode *startnode, *curr, *frontier, *sn, *submatch, *matches;
    unsigned begin, end, match;
    Node *n;

    matches = NULL;
    submatch = NULL;
    match = 0;

    for (begin = start; !match && begin < finish; ++begin) {
        for (end = finish; !match && end > begin; --end) {

            frontier = startnode = snode(s, begin, NULL);
            while (matches) {
                sn = matches;
                matches = matches->next;
                free(sn);
            }
            matches = snode(s, begin, NULL);

            while (frontier) {

                curr = frontier;
                frontier = NULL;

                while (curr) {
                    if (!match && curr->s == a && curr->i == end) {

                        n = node(a, NULL);
                        matches = append(matches, curr->i, n);
                        free(n);
                        match = 1;

                    } else if (!match && curr->s != a) { 

                        /* epsilon transitions */
                        append(curr, curr->i, curr->s->trans[EPSILON]);

                        /* matching transitions */
                        if (curr->i <= end) 
                            frontier = append(frontier, curr->i+1, curr->s->trans[(int)str[curr->i]]);

                        /* matching subexpressions */
                        if (curr->s->trans[LPAREN]) {
                            n = curr->s->trans[LPAREN];
                            submatch = search_rec(n->s, n->s->mate, str, curr->i, end);

                            if (submatch && n->s->mode == PLUS) {
                                    sn = submatch;
                                    while (sn->next) sn = sn->next;
                                    n = node(sn->s, NULL); /* get the matching right paren */
                                    frontier = append(frontier, sn->i, n);
                                    free(n);

                                    while(submatch) {
                                        n = node(submatch->s, NULL);
                                        matches = append(matches, submatch->i, n);
                                        free(n);

                                        sn = submatch;
                                        submatch = submatch->next;
                                        free(sn);
                                    }
                            } else if (!submatch && n->s->mode == MINUS) {
                                unsigned i;
                                n = node(n->s->mate, NULL);
                                for (i = curr->i+1; i < end; ++i)
                                    frontier = append(frontier, i, n);
                                free(n);
                            }
                        }
                    }
                    sn = curr;
                    curr = curr->next;
                    free(sn);
                }
            }

            if (a->mode == CASH) break;
        }

        if (s->mode == CARET) break;
    }

    if (!match) {
        while (matches) {
            sn = matches;
            matches = matches->next;
            free(sn);
        }
    }

    return matches;
}

unsigned search(State *s, State *a, char *str, MatchObject *m) {
    SNode *beg, *end, *match;
    
    match = search_rec(s, a, str, 0, str[0] ? strlen(str) : 1);
    m->groups = NULL;
    m->str = NULL;
    m->n = 0;
    if (m && match) {
        beg = match;
        m->str = str;
        while (beg) {
            end = beg;
            while (end && end->s != beg->s->mate)
                end = end->next;
            add_group(m, beg->i, end->i);
            do {
                beg = beg->next;
            } while (beg && !beg->s->mate);
        }
        m->n--;
        return 1;
    } else return 0;
}
