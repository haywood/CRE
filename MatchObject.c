#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "nfa.h"

void addGroup(MatchObject *m, int b, int e)
{
    Group newG, *oldG;
    int i;

    if (!m) return;

    oldG=m->groups;
    newG.gbeg=b;
    newG.gend=e;

    while (oldG != m->groups+m->n) {
        if (b < oldG->gbeg) {
            break;

        } else if (b == oldG->gbeg) {
            if (e > oldG->gend)
                break;

            else if (e == oldG->gend)
                return;

            else oldG++;

        } else oldG++;
    }

    i=oldG - m->groups;
    m->groups=(Group *)realloc(m->groups, ++m->n*sizeof(Group));
    if (!m->groups) {
        fprintf(stderr, "error: addGroup: out of memory\n");
        exit(1);
    }
    memmove(m->groups+i+1, m->groups+i, (m->n-i-1)*sizeof(Group));
    m->groups[i]=newG;
}

Group *group(int b, int e)
{
    Group *g = (Group *)malloc(sizeof(Group));
    g->gbeg = b;
    g->gend = e;
    return g;
}

MatchObject *matchObject(const char *str, int n, Group *g)
{
    MatchObject *m = (MatchObject *)malloc(sizeof(MatchObject));
    m->groups = g;
    m->str = str;
    m->n = n;
    return m;
}

