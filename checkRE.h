#ifndef CHECK_RE_H_
#define CHECK_RE_H_

#include <string.h>
#include <ctype.h>

#include "searchNFA.h"

#define END_ERROR "error: in checkRE, expected ')', quantifier or literal, but found %c\n"
#define BRACE_ERROR "error: in checkRE, malformed fixed repetition\n"

inline int checkBraces(const char *re, const char *reend)
{
    int error=0, atStart=1, hasComma=0, minReps, maxReps;

    if (!re || !*re || re == reend) return 1;

    minReps=atoi(re);

    while (!error && re != reend) {
        if (*re == ',') {
            if (atStart) error=1;

            else {
                hasComma=1;
                maxReps=atoi(re+1);
            }
        } else if (!isdigit(*re)) error=1;

        re++;
        atStart=0;
    }

    if (minReps <= 0 || (hasComma && minReps > maxReps))
        error=1;

    if (error) fprintf(stderr, BRACE_ERROR);

    return error;
}

/**
 * Check the syntax of a regular expression string.
 */
inline int checkRE(const char *re, const char *reend)
{
    int atStart=1, atEnd=0, paren=0, error=0;
    const char *subre;

    if (!re) return 0;
    if (!*re) return 1;

    while (re != reend) {

        atEnd=re+1 == reend;

        if (atStart && (*re == '*' || *re == '+' || *re == '?' || *re == '{')) {
                fprintf(stderr, "error: in checkRE, expected literal, '[', or '(', but found %c instead\n", *re);
                error=1;
        }

        if (!legalChar(*re)) {
            fprintf(stderr, "error: in checkRE, illegal control chracter: %c\n", *re);
            error=1;

        } else if (*re == '\\') {
            if (atEnd) {
                fprintf(stderr, END_ERROR, '\\');
                error=1;
            } 

            re++;

            if (!legalChar(*re)) {
                fprintf(stderr, "error: in checkRE, illegal control character: %c\n", *re);
                error=1;
            }

        } else if (*re == '~' && atEnd) {
            fprintf(stderr, END_ERROR, '~');
            error=1;

        } else if (*re == '(') {
            subre=++re;
            paren=1;

            while (paren && re != reend) {
                if (*re == '(') paren++;
                else if (*re == ')') paren--;
                if (paren && re != reend) re++;
            }

            if (re == reend) {
                fprintf(stderr, "error: in checkRE, encountered end of input while parsing subexpression\n");
                exit(1);
            } 
            
            error=error || checkRE(subre, re);
            re++;

        } else if (*re == '[') {
            subre=++re;
            while (re != reend && *re != ']') {
                if (*re == '\\') re++;
                if (re != reend) re++;
            }
            if (re == reend) {
                fprintf(stderr, "error: in checkRE, unmatched ']'\n");
                error=1;
            }

            if (re != reend) re++;
            else re = subre;

        } else if (*re == '{') {
            subre=++re;

            while (re != reend && *re != '}') re++;
            
            if (re == reend) {
                fprintf(stderr, "error: in checkRE, reached end of expression while searching for '}'\n");
                error=1;

            } else {
                error=error || checkBraces(subre, re-1);
                re++;

            }
        }

        if (re != reend) re++;
        atStart=0;
    }

    if (error) {
        fprintf(stderr, "exiting due to previous errors\n");
        exit(1);
    }

    return error;
}

#endif /* CHECK_RE_H_ */
