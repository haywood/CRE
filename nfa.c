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
    MatchObject m;
    unsigned i, k;
    Node *n, *t;
    NFA *d;

    if (argc > 2) {
        checkRE(argv[1]);
        d = buildNFA(argv[1], 0);
        printf("%s search %s = %d\n", argv[1], argv[2], 
                search(d->start, d->accept, argv[2], &m, d->flags & MATCHSTART, d->flags & MATCHEND));
        printf("%u capture groups:\n", m.n);
        if (m.groups) {
            for (i = 0; i <= m.n; ++i) {
                printf("%d, %d: ", m.groups[i].i[0], m.groups[i].i[1]);
                for (k = m.groups[i].i[0]; k < m.groups[i].i[1]; ++k)
                    putchar(m.str[k]);
                putchar('\n');
            }
        }
        n = d->start->trans[STATE_LIST];
        while ((t=n)) {
            free(n->s);
            n=n->next;
            free(t);
        }
        free(d->start);
        free(d->accept);
        free(d);
    }
    return 0;
}

