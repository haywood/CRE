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
    int s, mode;
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

typedef struct _group {
    unsigned int i[2];
    struct _group *next;
} group;

typedef struct _match_object {
    char *str;
    group *groups;
    unsigned int n;
} match_object;

/* allocate a state */
state * stalloc(int, state *, state *, int);

/* Search for a substring that the nfa accepts */
int search(nfa *, char *, unsigned int,  match_object *, int);

/* check is state represents an epsilon transition */
int epsilon(state *);

/* check if a state is accepting */
int accepting(state *);

/**
 * Consruct an nfa from a regular expression.
 * Precondition: the regular expression is legal.
 */
nfa * construct_nfa(char *);

void free_nfa(nfa *);
