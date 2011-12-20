#include <stdlib.h>
#include <string.h>

#include "nfa.h"

void addGroup(MatchObject *m, unsigned b, unsigned e)
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

