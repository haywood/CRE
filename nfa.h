/**
 * nfa.h
 */

typedef enum {
    DOTALL=1
} nfa_option;

typedef enum {
    ACCEPT=CHAR_MAX+1,
    DOT,
    EPSILON,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET
} nfa_symbol;

typedef struct _state {
    int s;
    struct _state *c[2];
} state;

typedef struct _node {
    state *s;
    struct _node *next;
} node;

typedef struct _nfa {
    state *start, *accept;
    node *states;
    int n;
} nfa;

// allocate a state
state * stalloc(int, state *, state *);

// Match a string against an nfa.
int accepts(state *, char *, int);

// Search for a substring that the nfa accepts
int search_accepts(state *, char *, int)

// check is state represents an epsilon transition
int epsilon(state *);

// check if a state is accepting
int accepting(state *);

/**
 * Consruct an nfa from a regular expression.
 * Precondition: the regular expression is legal.
 */
nfa * construct_nfa(char *);

void free_nfa(nfa *);
