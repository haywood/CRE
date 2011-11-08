/**
 * nfa.h
 */

#define N_KIDS 2

typedef enum {
    DOTALL=1
} nfa_option;

typedef enum {
    ACCEPT=CHAR_MAX+1,
    CASH,
    DOT,
    CARET,
    EPSILON,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET
} nfa_symbol;

typedef struct _state {
    int s;
    struct _state *c[N_KIDS];
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

typedef struct _result_node {
    state *s;
    int i;
    struct _result_node *parent, *next;
} result_node;

typedef struct _search_node {
    result_node *n;
    struct _search_node *next;
} search_node;

/* allocate a state */
state * stalloc(int, state *, state *);

/* Match a string against an nfa. */
int match(state *, char *, int);

/* Search for a substring that the nfa accepts */
int search(nfa *, char *, unsigned int, unsigned int **, int);

/* check is state represents an epsilon transition */
int epsilon(state *);

/* check if a state is accepting */
int accepting(state *);

/**
 * Consruct an nfa from a regular expression.
 * Precondition: the regular expression is legal.
 */
nfa * construct_nfa(char *);

node *make_node(state *, node *);

node *push(state *, node *);

state *pop(node **);

void append_result(state *, int, result_node *);

search_node *push_search(search_node *);

result_node *pop_search(search_node **);

result_node *push_result(state *, int , result_node *, result_node *);

state *pop_result(result_node **);

void free_nfa(nfa *);
