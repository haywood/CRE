/**
 * nfa.h
 */

#ifndef NFA_H_
#define NFA_H_

#include <limits.h>

#define PLUS 1
#define MINUS -1

#define WILDCARD '.'

enum {
    DOTALL=1 << 0,
    MATCHSTART=1 << 1,
    MATCHEND=1 << 2
};

enum {
    EPSILON,
    STATE_LIST=CHAR_MAX+1,
    NUM_SYMB
};

typedef struct _Group Group;
typedef struct _MatchObject MatchObject;
typedef struct _NFA NFA;
typedef struct _Node Node;
typedef struct _State State;

struct _Group {
    int i[2];
};

struct _MatchObject {
    const char *str;
    int n;
    Group *groups;
};

struct _NFA {
    State *start, 
          *accept;
    int flags;
};

struct _Node {
    State *s;
    struct _Node *next;
};

struct _State {
    int mode; /* context */
    Node *trans[NUM_SYMB];
    State *mate; /* matching state if a delimiter */
};

State *addChild(State *, int, State *);

void addGroup(MatchObject *, int, int);

Node *getChild(State *, int);

Group *group(int, int);

MatchObject *matchObject(const char *, int, Group *);

NFA *buildNFA(const char *, int);

Node *node(State *, Node *);

Node *popNode(Node *);

State *state(int, State *);

int search(State *, State *, const char *, MatchObject *, int);

#endif
