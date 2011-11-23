#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "nfa.h"

int isQuant(char c) { return c == '*' || c == '+' || c == '?'; }

void check_re(char *re)
{
    int bracket=0, paren=0, error=0;
    char *c=re;
    while (*c) {
        if (!isgraph(*c) && !isspace(*c)) {
            fprintf(stderr, "error: in check_re, illegal control chracter: %c\n", *c);
            error=1;
        }
        if (*c == '\\') {
            c++;
        } else if (*c == '|' && (*(c-1) == '(' || *(c+1) == ')' || c == re || !*(c+1))) {
            fprintf(stderr, "error: in check_re, misplaced %c\n", *c);
            error=1;
        } else if (*c == '~' && (*(c+1) == ')' || c == re || !*(c+1))) {
            fprintf(stderr, "error: in check_re, misplaced %c\n", *c);
            error=1;
        } else if (!bracket && (c == re || isQuant(*(c-1)) || *(c-1) == '(' ) && isQuant(*c)) {
            fprintf(stderr, "error: in check_re, expecting literal or delimiter, but found quantifier or operator instead: %c\n", *c);
            error=1;
        } else if (*c == '[') {
            if (!bracket) bracket=1;
        } else if (*c == ']') {
            if (!bracket) {
                fprintf(stderr, "error: in check_re, unmatched ]\n");
                error=1;
            } else bracket=0;
        } else if (*c == '(' && !bracket) {
            paren++;
        } else if (*c == ')' && !bracket) {
            if (paren) paren--;
            else {
                fprintf(stderr, "error: in check_re, unmatched )\n");
                error=1;
            }
        }
        c++;
    }
    if (bracket || paren) {
        if (bracket) fprintf(stderr, "error: in check_re, unmatched [\n");
        else if (paren) fprintf(stderr, "error: in check_re, unmatched (\n");
        error=1;
    }
    if (error) {
        fprintf(stderr, "exiting due to previous errors\n");
        exit(1);
    }
}