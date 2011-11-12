#include <stdlib.h>
#include <assert.h>
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

SNode *search_rec(State *s, State *a, char *str, unsigned int start, unsigned int finish, int options)
{
    SNode *startnode, *curr, *frontier, *sn, *submatch, *matches;
    unsigned begin, end, match;
    Node *n;
    int c;

    matches = NULL;
    submatch = NULL;
    match = 0;

    for (begin = start; !match && begin < finish; ++begin) {
        for (end = finish; !match && end > begin; --end) {

            frontier = startnode = snode(s, start, NULL);
            while (matches) {
                sn = matches;
                matches = matches->next;
                free(sn);
            }
            matches = snode(s, start, NULL);

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

                        append(curr, curr->i, curr->s->trans[EPSILON]);

                        frontier = append(frontier, curr->i+1, curr->s->trans[(int)str[curr->i]]);

                        if (curr->s->trans[LPAREN]) {
                            n = curr->s->trans[LPAREN];
                            submatch = search_rec(n->s, n->s->mate, str, curr->i, end, options);

                            if (submatch) {
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

unsigned search(State *s, State *a, char *str, unsigned int start, unsigned int finish, MatchObject *m, int options) {
    SNode *beg, *end, *match;
    
    match = search_rec(s, a, str, start, finish, options);
    if (match) {
        beg = match;
        while (beg) {
            end = beg;
            while (end && end->s != beg->s->mate)
                end = end->next;
            add_group(m, beg->i, end->i);
            do {
                beg = beg->next;
            } while (beg && !beg->s->mate);
        }
        return 1;
    } else return 0;
}
