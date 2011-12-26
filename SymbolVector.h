#ifndef SYM_VEC_H_
#define SYM_VEC_H_

#include <string.h>
#include <limits.h>
#include <ctype.h>

#define EPSILON 0

#define INT_IN_BITS ((signed)(8*sizeof(int)))

#define RE_SYM_VEC_LEN ((signed)((CHAR_MAX+1)/INT_IN_BITS))

typedef int SymbolVector[RE_SYM_VEC_LEN];

inline int legalChar(char c) { return isspace(c) || isgraph(c); }

int isCharClass(char c)
{
    c = tolower(c);
    return c == 'w' || c == 'd';
}

int isCharClassMember(char c, char classCode)
{
    int isMember;
    switch (tolower(classCode)) {
        case 'w':
            isMember=isalnum(c) || c == '_';
            break;
        case 'd':
            isMember=isdigit(c);
            break;
        default:
            return 0;
    }
    return islower(classCode) ? isMember : !isMember;
}

inline SymbolVector *symbolVector()
{
    SymbolVector *symVec=(SymbolVector *)malloc(sizeof(SymbolVector));

    memset(symVec, 0, sizeof(SymbolVector));

    return symVec;
}

inline void addSymbol(SymbolVector *symVec, char symbol)
{
    int index, bit;

    if (!symVec) return;

    index=symbol/INT_IN_BITS;
    bit=symbol%INT_IN_BITS;

    if (index >= RE_SYM_VEC_LEN) return;

    (*symVec)[index]|= 1 << bit;
}

inline void removeSymbol(SymbolVector *symVec, char symbol)
{
    int index, bit;

    if (!symVec) return;

    index=symbol/INT_IN_BITS;
    bit=symbol%INT_IN_BITS;

    if (index >= RE_SYM_VEC_LEN) return;

    (*symVec)[index]&= ~(1 << bit);
}

inline int symVecContains(SymbolVector *symVec, char symbol)
{
    int index, bit;

    if (!symVec) return 0;

    index=symbol/INT_IN_BITS;
    bit=symbol%INT_IN_BITS;

    if (index >= RE_SYM_VEC_LEN) return 0;

    return (*symVec)[index] & (1 << bit);
}

inline void symVecCompliment(SymbolVector *sv, SymbolVector *res)
{
    int index;
    for (index=0; index < RE_SYM_VEC_LEN; ++index)
        (*res)[index]=~(*sv)[index];
}

inline void symVecAnd(SymbolVector *sv1, SymbolVector *sv2, SymbolVector *res)
{
    int index;

    for (index=0; index < RE_SYM_VEC_LEN; ++index)
        (*res)[index]=(*sv1)[index] & (*sv2)[index];
}

inline void symVecOr(SymbolVector *sv1, SymbolVector *sv2, SymbolVector *res)
{
    int index;

    for (index=0; index < RE_SYM_VEC_LEN; ++index)
        (*res)[index]=(*sv1)[index] | (*sv2)[index];
}

inline void symVecXor(SymbolVector *sv1, SymbolVector *sv2, SymbolVector *res)
{
    int index;

    for (index=0; index < RE_SYM_VEC_LEN; ++index)
        (*res)[index]=(*sv1)[index] ^ (*sv2)[index];
}

inline void symVecClear(SymbolVector *symVec)
{
    memset(symVec, 0, sizeof(SymbolVector));
}

inline SymbolVector *epsilonVector()
{
    SymbolVector *eps=symbolVector();
    addSymbol(eps, EPSILON);
    return eps;
}

#endif /* SYM_VEC_H_ */
