/**
 * nfa.h
 */

#ifndef NFA_H_
#define NFA_H_

#include <limits.h>

#define PLUS 1
#define MINUS -1

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
    unsigned int i[2];
    struct _Group *next;
};

struct _MatchObject {
    char *str;
    unsigned int n;
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

void addGroup(MatchObject *, unsigned, unsigned);

void checkRE(char *);

Node *getChild(State *, int);

Group *group(unsigned int, unsigned, Group *);

MatchObject *matchObject(char *, unsigned, Group *);

NFA *buildNFA(const char *, int);

Node *node(State *, Node *);

Node *popNode(Node *);

State *state(int, State *);

unsigned search(State *, State *, char *, MatchObject *, int, int);

#endif
