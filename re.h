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

inline int rereplace(RE *, char *, char *)
{
    return 0;
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
