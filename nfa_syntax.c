#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "nfa.h"

int followedOp(char c) { return c == '~' || c == '|'; }
int precededOp(char c) { return c == '*' || c == '+' || c == '?'; }

void check_re(char *re)
{
    int bracket=0, paren=0;
    char *c=re;
    while (*c) {
        if (!isgraph(*c) && !isspace(*c)) {
            fprintf(stderr, "error: in check_re, illegal control chracter: %c\n", *c);
            abort();
        }
        if (*c == '\\') {
            c++;
        } else if (followedOp(*c) && (!*(c+1) || *(c+1) == ')' || (*(c+1) == '$' && !*(c+2)))) {
            fprintf(stderr, "error: in check_re, misplaced %c\n", *c);
            abort();
        } else if (!bracket && (c == re || precededOp(*(c-1))) && precededOp(*c)) {
            fprintf(stderr, "error: in check_re, expecting literal or delimiter, but found quantifier instead: %c\n", *c);
            abort();
        } else if (*c == '[') {
            if (!bracket) bracket=1;
        } else if (*c == ']') {
            if (!bracket) {
                fprintf(stderr, "error: in check_re, unmatched ]\n");
                abort();
            } else bracket=0;
        } else if (*c == '(' && !bracket) {
            paren++;
        } else if (*c == ')' && !bracket) {
            if (paren) paren--;
            else {
                fprintf(stderr, "error: in check_re, unmatched )\n");
                abort();
            }
        }
        c++;
    }
    if (bracket || paren) {
        if (bracket) fprintf(stderr, "error: in check_re, unmatched [\n");
        else if (paren) fprintf(stderr, "error: in check_re, unmatched (\n");
        abort();
    }
}
