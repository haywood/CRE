#include <string.h>
#include <stdlib.h>

#include "nfa.h"

NFA *compileRE(const char *re)
{
    int flags=0, reLen=strlen(re);
    char *beg=re, *end=re+reLen;
    if (*re=='^') {
        flags|=MATCHSTART;
        beg++;
    }
    if (re[reLen-1]=='$') {
        flags|=MATCHEND;
        end--;
    }
    char *recpy=(char *)calloc((1+(end-beg)), sizeof(char));
    memcpy(recpy, beg, end-beg);
    return buildNFA(recpy, flags);
}
