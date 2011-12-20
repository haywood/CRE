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

SNode *search_rec(State *s, State *a, const char *str, unsigned int start, unsigned int finish, int flags)
{
    SNode *startnode, *curr, *frontier, *sn, *pn, *matches, *lists;
    int begin, end, alive, matchstart, matchend;
    Node *n;

    matchstart=flags & MATCHSTART;
    matchend=flags & MATCHEND;

    alive = 1;

    for (begin = start; alive && begin <= finish; ++begin) {
        for (end = finish; alive && end >= begin; --end) {

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

                    } else if (alive && curr->s != s && curr->s->mate && curr->s->mode == MINUS) {
                        
                        /* handle a negated parenthetical */
                        
                        sn = search_rec(curr->s, curr->s->mate, str, begin, end, 0);
                        if (!sn) {
                            append(curr, curr->i, curr, curr->s->trans[EPSILON]);
                            if (curr->i <= end) frontier = append(frontier, curr->i+1, curr, curr->s->trans[(int)str[curr->i]]);
                        }
                        
                    } else if (alive && curr->s != a) { 

                        /* epsilon transitions */
                        append(curr, curr->i, curr, curr->s->trans[EPSILON]);

                        /* matching transitions */
                        if (curr->i < end) frontier = append(frontier, curr->i+1, curr, curr->s->trans[(int)str[curr->i]]);

                    }
                } /* curr loop */
            } /* while frontier */

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

unsigned search(State *s, State *a, const char *str, MatchObject *m, int flags) {

    SNode *beg, *end, *match, *sn;
    
    match = search_rec(s, a, str, 0, strlen(str), flags);
    if (m) {
        m->groups = NULL;
        m->str = NULL;
        m->n = 0;
    }
    if (match) {
        puts("matched");
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
            puts("calculating groups");
            m->str = str;
            beg = match;
            while (beg->s != s) beg = beg->parent;
            while (beg->s != a) {
                if (beg->s->mate) {
                    end = beg->next;
                    while (end && end->s != beg->s->mate)
                        end = end->next;
                    addGroup(m, beg->i, end->i);
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
