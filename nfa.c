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
    NFA *d;

    if (argc > 2) {
        d = nfa(argv[1]);
        m.str = argv[2];
        m.groups = NULL;
        m.n = 0;
        printf("%s search %s = %d\n", argv[1], argv[2], 
                search(d->start, d->accept, argv[2], &m));
        printf("%d groups:\n", m.n);
        for (i = 0; i < m.n; ++i) {
            for (k = m.groups[i].i[0]; k < m.groups[i].i[1]; ++k)
                putchar(m.str[k]);
            putchar('\n');
        }
        printf("Match empty: %d\n", d->empty);
    }
    return 0;
}

