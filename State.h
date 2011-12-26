#ifndef STATE_H_
#define STATE_H_

#include "SymbolVector.h"

#define WILDCARD '.'

enum {
    DOTALL=1 << 0,
    MATCHSTART=1 << 1,
    MATCHEND=1 << 2,
    ICASE=1 << 3,
    FINDALL=1 << 4
};

enum {
    NONE=0,
    LPAREN=1 << 0,
    RPAREN=1 << 1,
    MINUS=1 << 2
};

typedef struct _State State;
typedef struct _Node Node;

struct _State {
    State *mate, *lchild, *rchild;
    SymbolVector *symbols;
    Node *states;
    int mode;
};

struct _Node {
    State *s;
    Node *next;
};

inline Node *pushNode(Node *nodes, State *s)
{
    Node *n=(Node *)malloc(sizeof(Node));
    n->next=nodes;
    n->s=s;
    return n;
}

inline Node *popNode(Node *nodes)
{
    Node *n=nodes;
    if (n) nodes=nodes->next;
    free(n);
    return nodes;
}

inline State *state(SymbolVector *symbols, int mode, State *mate)
{
    State *s=(State *)malloc(sizeof(State));
    s->lchild=s->rchild=NULL;
    s->symbols=symbols;
    s->states=NULL;
    s->mode=mode;
    s->mate=mate;

    return s;
}

inline void addState(State *prnt, State *child)
{
    prnt->states=pushNode(prnt->states, child);
}

inline void addStates(State *prnt, Node *states)
{
    Node *front=states;
    if (prnt && states) {
        while (front) {
            front=front->next;
            states->next=prnt->states;
            prnt->states=states;
            states=front;
        }
    }
}

#endif /* STATE_H_ */
