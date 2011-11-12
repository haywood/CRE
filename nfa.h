/**
 * nfa.h
 */

#ifndef NFA_H_
#define NFA_H_

#include <limits.h>

#define PLUS 1
#define MINUS -1
#define CARET 2
#define CASH 3

typedef enum {
    DOTALL=1
} NFA_option;

typedef enum {
    EPSILON,
    LPAREN=CHAR_MAX+1,
    RPAREN,
    LBRACK,
    RBRACK,
    NUM_SYMB
} NFA_symbol;

typedef struct _State State;

typedef struct _NFA {
    State *start, 
          *accept;
    int empty; /* if matches empty */
} NFA;

typedef struct _Node {
    State *s;
    struct _Node *next;
} Node;

struct _State {
    int mode; /* if match or negation */
    Node *trans[NUM_SYMB];
    State *mate; /* matching state if a delimiter */
};

typedef struct _Group {
    unsigned int i[2];
    struct _Group *next;
} Group;

typedef struct _MatchObject {
    char *str;
    unsigned int n;
    Group *groups;
} MatchObject;

State *state(int, State *);

Node *node(State *, Node *);

Node *pop(Node *);

Group *group(unsigned int, unsigned, Group *);

MatchObject *matchObject(char *, unsigned, Group *);

void add_group(MatchObject *, unsigned, unsigned);

NFA *nfa(char *);

State *add_child(State *, int, State *);

unsigned search(State *, State *, char *, unsigned, unsigned, MatchObject *, int);

#endif
