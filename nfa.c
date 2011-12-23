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
    int matched, i, k;
    char *str, *token;
    RE *re;

    if (argc > 2) {
        re = compileRE(argv[1], 0);

        str = (char *)calloc(1+strlen(argv[2]), sizeof(char));
        strcpy(str, argv[2]);

        memset(&m, 0, sizeof(MatchObject));
        matched=rematch(re, str, &m);
        printf("%s search %s = %d\n", re->restr, str, matched);
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
        rereplace(re, &str, ":)", 1);
        printf("%s\n", str);

        free(str);
        str=argv[2];
        token=NULL;

        while (resep(re, &str, &token)) {
            printf("%s\n", token);
        }
        freere(re);
    }
    return 0;
}

