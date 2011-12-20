#include <stdlib.h>
#include <string.h>

#include "nfa.h"

void addGroup(MatchObject *m, int b, int e)
{
    int i, swapped;
    Group g[2];

    if (!m) return;

    i=m->n;
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

Group *group(int b, int e)
{
    Group *g = (Group *)malloc(sizeof(Group));
    g->i[0] = b;
    g->i[1] = e;
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

