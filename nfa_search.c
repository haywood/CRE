#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "nfa.h"

typedef struct _SNode {
    State *s;
    unsigned i;
    struct _SNode *parent,
                  *next;
} SNode;

SNode *snode(State *s, unsigned i, SNode *parent, SNode *next)
{
    SNode *n = (SNode *)malloc(sizeof(SNode));
    n->s = s;
    n->i = i;
    n->parent = parent;
    n->next = next;
    return n;
}

SNode *append(SNode *l, unsigned i, SNode *parent, Node *n) 
{
    SNode *tail = l;
    while (tail && tail->next) {
        tail = tail->next;
    }
    while (n) {
        if (tail) {
            tail = tail->next = snode(n->s, i, parent, NULL);
        }
        else l = tail = snode(n->s, i, parent, NULL);
        n = n->next;
    }
    return l;
}

SNode *search_rec(State *s, State *a, char *str, unsigned int start, unsigned int finish, int matchstart, int matchend)
{
    SNode *startnode, *curr, *frontier, *sn, *pn, *matches, *lists;
    unsigned begin, end, alive;
    Node *n;

    alive = 1;

    for (begin = start; alive && begin < finish; ++begin) {
        for (end = finish; alive && end > begin; --end) {

            frontier = startnode = snode(s, begin, NULL, NULL);
            matches = NULL;
            lists = NULL;

            while (frontier) {

                sn = lists; /* save the old frontier */
                if (sn) {
                    while (sn->next) 
                        sn = sn->next;
                    sn->next = frontier;
                } else lists = frontier;

                curr = frontier;
                frontier = NULL;

                for (; alive && curr; curr = curr->next) {
                    if (alive && curr->s == a && curr->i == end) {

                        n = node(a, NULL);
                        sn=matches=snode(curr->s, end, NULL, matches);
                        pn=curr->parent;
                        while (pn) {
                            sn=sn->parent=snode(pn->s, pn->i, NULL, sn);
                            pn=pn->parent;
                        }
                        free(n);
                        alive = 0;

                    } else if (alive && curr->s != a) { 

                        /* epsilon transitions */
                        append(curr, curr->i, curr, curr->s->trans[EPSILON]);

                        /* matching transitions */
                        if (curr->i <= end) 
                            frontier = append(frontier, curr->i+1, curr, curr->s->trans[(int)str[curr->i]]);

                        /* parentheticals */
                        if (curr->s->trans[LPAREN]) {
                            if(curr->s->trans[LPAREN]->s->mode == PLUS)
                                frontier = append(frontier, curr->i, curr, curr->s->trans[LPAREN]);
                            else {
                                sn = search_rec(curr->s->trans[LPAREN]->s, curr->s->trans[LPAREN]->s->mate, str, begin, end, 0, 0);
                                if (sn) {
                                    while ((curr=frontier)) {
                                        frontier = frontier->next;
                                        free(curr);
                                    }
                                    while ((curr=sn)) {
                                        sn = sn->next;
                                        while ((pn=curr)) {
                                            curr=curr->parent;
                                            free(pn);
                                        }
                                    }
                                    alive = 0;
                                    break;
                                } else {
                                    n = node(curr->s->trans[LPAREN]->s->mate, NULL);
                                    frontier = append(frontier, curr->i, curr, n);
                                    free(n);
                                }
                            }
                        }
                    }
                }
            }

            while ((curr=lists)) {
                lists=lists->next;
                free(curr);
            }

            if (matchend) break;
        }

        if (matchstart) break;
    }

    return matches;
}

unsigned search(State *s, State *a, char *str, MatchObject *m, int matchstart, int matchend) {

    SNode *beg, *end, *match, *sn;
    
    match = search_rec(s, a, str, 0, str[0] ? strlen(str) : 1, matchstart, matchend);
    if (m) {
        m->groups = NULL;
        m->str = NULL;
        m->n = 0;
    }
    if (match) {
        if (match->next) { /* this should never happen, but if somehow it does... */
            end = match->next;
            while ((beg=end)) {
                end = end->next;
                while ((sn=beg)) {
                    beg = beg->parent;
                    free(sn);
                }
            }
        }
        if (m) {
            m->str = str;
            beg = match;
            while (beg->s != s) beg = beg->parent;
            add_group(m, beg->i, match->i);
            while (beg->s != a) {
                if (beg->s->trans[LPAREN] && beg->next->s == beg->s->trans[LPAREN]->s) {
                    beg = beg->next;
                    end = beg->next;
                    while (end && end->s != beg->s->mate)
                        end = end->next;
                    add_group(m, beg->i, end->i);
                }
                sn=beg;
                beg=beg->next;
                beg->parent=NULL;
                free(sn);
            }
            m->n--;
        }
        return 1;
    } else return 0;
}
