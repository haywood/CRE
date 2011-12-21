#ifndef RE_H_
#define RE_H_

#include "nfa.h"

typedef struct _RE RE;

struct _RE {
    char *restr;
    NFA *nfa;
};


/**
 * Check the syntax of a regular expression string.
 */
inline void checkRE(char *re)
{
    int bracket=0, paren=0, error=0;
    char *c=re;

#define isQuant(c) ( (c) == '*' || (c) == '+' || (c) == '?' )
#define atStart ( c-1 == re || *(c-1) == '(' )
#define atEnd ( *(c+1) == '\0' || *(c+1) == ')' )

    while (*c) {
        if (!isgraph(*c) && !isspace(*c)) {
            fprintf(stderr, "error: in checkRE, illegal control chracter: %c\n", *c);
            error=1;

        }
        if (*c == '\\') {
            c++;

        } else if (*c == '|' && (atStart || atEnd)) {
            fprintf(stderr, "error: in checkRE, misplaced %c\n", *c);
            error=1;

        } else if (*c == '~' && (c-1 == re || atEnd)) {
            fprintf(stderr, "error: in checkRE, misplaced %c\n", *c);
            error=1;

        } else if (!bracket && (atStart || isQuant(*(c-1))) && isQuant(*c)) {
            fprintf(stderr, "error: in checkRE, expecting literal or delimiter, but found quantifier or operator instead: %c\n", *c);
            error=1;

        } else if (*c == '[') {
            if (!bracket) bracket=1;

        } else if (*c == ']') {
            if (!bracket) {
                fprintf(stderr, "error: in checkRE, unmatched ]\n");
                error=1;
            } else bracket=0;

        } else if (*c == '(' && !bracket) {
            paren++;

        } else if (*c == ')' && !bracket) {
            if (paren) paren--;
            else {
                fprintf(stderr, "error: in checkRE, unmatched )\n");
                error=1;
            }
        }
        c++;
    }
    if (bracket || paren) {
        if (bracket) fprintf(stderr, "error: in checkRE, unmatched [\n");
        else if (paren) fprintf(stderr, "error: in checkRE, unmatched (\n");
        error=1;
    }
    if (error) {
        fprintf(stderr, "exiting due to previous errors\n");
        exit(1);
    }
}

/**
 * Compile an RE struct from restr.
 * restr is a pointer to a regular expression, which is checked for correctness by the function.
 * flags is a combination of compilation options. Currently only DOTALL to make the '.' operator match whitespace is supported.
 * The return value is a pointer to the compiled RE.
 */
inline RE *compileRE(const char *restr, int flags)
{
    const char *beg, *end;
    char *recpy;
    int reLen;
    RE *re;

    if (!restr) return 0;

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

/**
 * Check if re matches str and store the results in m.
 * re is a pointer to an RE.
 * str is a null-terminated string.
 * m is a pointer to a MatchObject.
 * Returns 1 if there is a  match and zero otherwise.
 * If there is a match, then the contents of m are overriden. Otherwise, m is unmodified.
 */
inline int rematch(RE *re, const char *str, MatchObject *m)
{
    if (!(re && str)) return 0;
    return search(re->nfa->start, re->nfa->accept, str, m, re->nfa->flags);
}

/**
 * Replace substrings of *str matching re with tepl.
 * re is a pointer to an RE.
 * str is a pointer to a dynamically allocated, null-terminated string.
 * repl is a null-terminated string.
 * if replaceAll is zero, then only the first instance of repl is replaced. Otherwise, all are replaced.
 * The function resizes *str as necessary.
 * *str remains null-terminated.
 */

inline int rereplace(RE * re, char **str, const char *repl, int replaceAll)
{
    int replLen, strLen, matchLen;
    int keepGoing, pos, count, lenChange;
    MatchObject m;
    Group *grp0;
    char *matchBegin, *matchEnd;

    if (!re && str && *str && repl) return 0;

    replLen=strlen(repl);
    strLen=strlen(*str);
    count=0;

    keepGoing=1;
    matchBegin=*str;

    memset(&m, 0, sizeof(MatchObject));

    while (keepGoing && *matchBegin && rematch(re, matchBegin, &m)) {
        grp0=m.groups;
        matchEnd=matchBegin+grp0->i[1];
        matchBegin=matchBegin+grp0->i[0];
        matchLen=matchEnd-matchBegin;
        pos=matchBegin-*str;

        lenChange=replLen-matchLen;

        /* if shrinking move the rest of the string to the left before resizing */
        if (replLen < matchLen) memmove(matchBegin+replLen, matchEnd, strlen(matchEnd));

        if (lenChange != 0) {
            strLen=strLen+lenChange; /* calculate new length of string */
            *str=(char *)realloc(*str, (1+strLen)*sizeof(char)); /* reallocate */
            if (!*str) {
                fprintf(stderr, "rereplace: out of memory\n");
                exit(1);
            }

            (*str)[strLen]='\0'; /* null terminate */
            matchBegin=*str+pos; /* make sure matchBegin moves with *str */
            matchEnd=matchBegin+matchLen;
        }

        /* if growing move the rest of the string to the right after resizing */
        if (replLen > matchLen) memmove(matchBegin+replLen, matchEnd, strlen(matchEnd));

        memcpy(matchBegin, repl, replLen); /* insert the replacement */
        matchBegin+=replLen; /* advance matchBegin to end of replacement substring */
        keepGoing=replaceAll;
        count++;
    }

    return count;
}

/**
 * Return a pointer to the start of the first substring of *str that re does not match. 
 * Advance *str past the first substring that re does match.
 * If no matching substring is found, returns the original value of *str while advancing *str to its end.
 * The substring matched by re is overriden with null characters as *str is advanced past it.
 */
inline char *resep(RE *re, char **str)
{
    MatchObject match;
    int matchLen;
    char *token;
    Group *grp0;

    if (!re || !str || !*str || !**str) return NULL;

    memset(&match, 0, sizeof(MatchObject));

    do {
        token=*str;
        if (rematch(re, *str, &match)) {
            grp0=match.groups;
            matchLen=grp0->i[1] - grp0->i[0];

            /* fill the area of the match with null terminators */
            *str+=grp0->i[0]; /* advance to start of match */
            while (matchLen--) *(*str)++='\0'; /* pad the match */

        } else while (**str) (*str)++;
    } while (!*token && **str);

    return token;
}

/**
 * Free a RE pointer and its resources.
 */
inline void freere(RE *re)
{
    Node *n, *t;

    if (!re) return;
    if (re->nfa) {
        n=getChild(re->nfa->start, STATE_LIST);
        while ((t=n)) {
            free(n->s);
            n=n->next;
            free(t);
        }
        free(re->nfa->start);
        free(re->nfa->accept);
        free(re->nfa);
    }
    free(re->restr);
    free(re);
}

#endif /* RE_H_ */
