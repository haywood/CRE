#ifndef RE_H_
#define RE_H_

#include "nfa.h"

typedef struct _RE RE;

struct _RE {
    char *restr;
    NFA *nfa;
};

inline RE *compileRE(const char *restr, int flags)
{
    const char *beg, *end;
    char *recpy;
    int reLen;
    RE *re;

    re=(RE *)malloc(sizeof(RE));
    reLen=strlen(restr);
    beg=restr;
    end=restr+reLen;

    if (*restr=='^') {
        flags|=MATCHSTART;
        beg++;
    }

    if (restr[reLen-1]=='$') {
        flags|=MATCHEND;
        end--;
    }

    recpy=(char *)calloc((1+(end-beg)), sizeof(char));
    memcpy(recpy, beg, end-beg);

    re->nfa=buildNFA(recpy, flags);
    re->restr=(char *)calloc((1+reLen), sizeof(char));
    memcpy(re->restr, restr, reLen);

    return re;
}

inline int rematch(RE *re, const char *str, MatchObject *m)
{
    if (!(re && str)) return 0;
    return search(re->nfa->start, re->nfa->accept, str, m, re->nfa->flags);
}

inline int rereplace(RE * re, char **str, const char *repl, int replaceAll)
{
    int replLen, strLen, matchLen;
    int keepGoing, pos, count;
    MatchObject m;
    Group *grp0;
    char *beg, *end;

    replLen=strlen(repl);
    strLen=strlen(*str);
    count=0;

    keepGoing=1;
    beg=*str;

    while (keepGoing && *beg && rematch(re, beg, &m)) {
        grp0=m.groups;
        end=beg+grp0->i[1];
        beg=beg+grp0->i[0];
        matchLen=end-beg;
        pos=beg-*str;

        if (matchLen < replLen) { /* str needs to grow */
            strLen=strLen+(replLen-matchLen); /* calculate new length of string */

            *str=(char *)realloc(*str, (1+strLen)*sizeof(char)); /* reallocate */
            if (!*str) {
                fprintf(stderr, "rereplace: out of memory\n");
                exit(1);
            }

            beg=*str+pos;
            (*str)[strLen]='\0'; /* null terminate */
            memmove(beg+replLen, end, strlen(end)); /* move rest of string over to make space for replacement */
            memcpy(beg, repl, replLen); /* insert the replacement */

        } else {
            memcpy(beg, repl, replLen);
            /* if replacement shorter than match */
            if (replLen < matchLen) { /* str needs to shrink */
                memmove(beg+replLen, end, strlen(end)); /* move rest of str to beg */
                strLen=strLen-(matchLen-replLen); /* calculate new length of string */

                *str=(char *)realloc(*str, (1+strLen)*sizeof(char)); /* reallocate */
                if (!*str) {
                    fprintf(stderr, "rereplace: out of memory\n");
                    exit(1);
                }

                (*str)[strLen]='\0'; /* null terminate */
                beg=*str+pos;
            }
        }

        keepGoing=replaceAll;
        beg+=replLen; /* advance beg to end of replacement substring */
        count++;
    }

    return count;
}

inline void freere(RE *re)
{
    Node *n, *t;
    n=getChild(re->nfa->start, STATE_LIST);
    while ((t=n)) {
        free(n->s);
        n=n->next;
        free(t);
    }
    free(re->nfa->start);
    free(re->nfa->accept);
    free(re->nfa);
    free(re->restr);
    free(re);
}

#endif /* RE_H_ */
