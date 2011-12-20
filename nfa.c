/**
 * nfa.c
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>

#include "re.h"

int main(int argc, char **argv)
{
    MatchObject m;
    unsigned i, k;
    int matched;
    RE *re;

    if (argc > 2) {
        re = compileRE(argv[1], 0);
        matched=rematch(re, argv[2], &m);
        printf("%s search %s = %d\n", argv[1], argv[2], matched);
        printf("%u capture groups:\n", m.n);
        if (m.groups) {
            for (i = 0; i <= m.n; ++i) {
                printf("%d, %d: ", m.groups[i].i[0], m.groups[i].i[1]);
                for (k = m.groups[i].i[0]; k < m.groups[i].i[1]; ++k)
                    putchar(m.str[k]);
                putchar('\n');
            }
        }
        free(m.groups);
        freere(re);
    }
    return 0;
}

