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
    int matched, i, k, flags;
    char *str, *token;
    RE *re;

    if (argc > 2) {
        flags=0;
        re = compileRE(argv[1], flags);

        str = (char *)calloc(1+strlen(argv[2]), sizeof(char));
        strcpy(str, argv[2]);

        memset(&m, 0, sizeof(MatchObject));
        matched=rematch(re, str, &m);
        printf("%s search %s = %d\n", re->restr, str, matched);
        printf("%u capture groups:\n", m.n);
        if (m.groups) {
            for (i = 0; i <= m.n; ++i) {
                printf("%d, %d: ", m.groups[i].gbeg, m.groups[i].gend);
                for (k = m.groups[i].gbeg; k < m.groups[i].gend; ++k)
                    putchar(m.str[k]);
                putchar('\n');
            }
        }

        puts("testing rereplace");
        free(m.groups);
        rereplace(re, &str, ":)", 1);
        printf("%s\n", str);

        free(str);
        str=argv[2];
        token=NULL;

        puts("testing resep");
        while (resep(re, &str, &token)) {
            printf("%s\n", token);
        }
        freere(re);
    }
    return 0;
}

