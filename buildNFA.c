#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "nfa.h"

int legalChar(int c) { return isgraph(c) || isspace(c); }

State *addChild(State *s, int t, State *c)
{
    if (!s || !c) return NULL;
    s->trans[t] = node(c, s->trans[t]);
    return c;
}

void addState(NFA *nfa, State *s)
{
    addChild(nfa->start, STATE_LIST, s);
}

void appendChildren(State *s0, int t, State *s1)
{
    Node *n;

    if (!(s0 && s1)) return;

    if (s0->trans[t]) {
        n=s0->trans[t];
        while (n->next) n=n->next;
        n->next=s1->trans[t];
    } else s0->trans[t]=s1->trans[t];
}

Node *getChild(State *s, int t)
{
    if (!s) return NULL;
    return s->trans[t];
}

int isCharClass(int c)
{
    c = tolower(c);
    return c == 'w' || c == 'd';
}

int isCharClassMember(char c, int classCode)
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

Node *node(State *s, Node *n)
{
    Node *no = (Node *)malloc(sizeof(Node));
    no->s = s;
    no->next = n;
    return no;
}

Node *popNode(Node *n)
{
    Node *o = n;
    if (o) {
        n = n->next;
        free(o);
    }
    return n;
}

State *state(int mode, State *mate)
{
    State *s = (State *)malloc(sizeof(State));
    s->mode = mode;
    s->mate = mate;
    memset(s->trans, 0, NUM_SYMB*sizeof(Node *));
    return s;
}

