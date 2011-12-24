#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "nfa.h"

typedef struct _SNode {
    State *s;
    int i;
    struct _SNode *parent,
                  *next;
} SNode;

SNode *snode(State *s, int i, SNode *parent, SNode *next)
{
    SNode *n = (SNode *)malloc(sizeof(SNode));
    n->s = s;
    n->i = i;
    n->parent = parent;
    n->next = next;
    return n;
}

SNode *pushSNode(SNode *l, unsigned i, SNode *parent, Node *n) 
{
    while (n) {
        l=snode(n->s, i, parent, l);
        n = n->next;
    }
    return l;
}

SNode *search_rec(State *s, State *a, const char *str, int start, int finish, int flags)
{
    SNode *curr, *frontier, *sn, *pn, *matches, *closedList;
    int alive, matchstart, matchend, i;

    matchstart=flags & MATCHSTART;
    matchend=flags & MATCHEND;

    alive = 1;

    frontier = NULL;
    for (i=finish; i >= start; --i)
        frontier=snode(s, i, NULL, frontier);
    matches = NULL;
    closedList = NULL;

    while (frontier) {

        curr = frontier;
        frontier = frontier->next;
        curr->next=closedList;
        closedList=curr;

        if (alive) {
            if (curr->s == a && (!matchend || curr->i == finish)) {

                sn=matches=snode(curr->s, curr->i, NULL, matches);
                pn=curr->parent;
                while (pn) {
                    sn=sn->parent=snode(pn->s, pn->i, NULL, sn);
                    pn=pn->parent;
                }

                if (matchstart && sn->i != start) {
                    sn=matches;
                    matches=matches->next;
                    while (sn) {
                        pn=sn->parent;
                        free(sn);
                        sn=pn;
                    }
                }

                alive = 0;

            } else if (curr->s != s && (curr->s->mode & LPAREN) && (curr->s->mode & MINUS)) {

                /* handle a negated parenthetical */

                sn = search_rec(curr->s, curr->s->mate, str, curr->i, finish, 0);
                if (!sn) frontier = snode(curr->s->mate, curr->i, curr, frontier);

            } else if (curr->s != a) { 

                /* epsilon transitions */
                frontier = pushSNode(frontier, curr->i, curr, getChild(curr->s, EPSILON));

                /* matching transitions */
                if (curr->i < finish) 
                    frontier = pushSNode(frontier, curr->i+1, curr, getChild(curr->s, str[curr->i]));

            }
        }
    } /* while frontier */

    while ((curr=closedList)) {
        closedList=closedList->next;
        free(curr);
    }

    return matches;
}

int search(State *s, State *a, const char *str, MatchObject *m, int flags) {

    SNode *beg, *end, *match, *sn;
    int paren, lparen, rparen;
    
    match = search_rec(s, a, str, 0, strlen(str), flags);
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

            free(m->groups);
            m->groups=NULL;
            m->n = 0;

            m->str = str;
            beg = match;

            while (beg->s != s) beg = beg->parent;

            addGroup(m, beg->i, match->i);

            while (beg->s != a) {
                lparen=beg->s->mode & LPAREN;
                if (lparen) { /* left paren */
                    end = beg->next;
                    paren=1;
                    while (paren) { /* find match */
                        lparen=end->s->mode & LPAREN;
                        rparen=end->s->mode & RPAREN;
                        if (lparen) paren++;
                        else if (rparen) paren--;
                        if (paren) end = end->next;
                    }
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
