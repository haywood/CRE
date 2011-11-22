CC=gcc
CFLAGS=-g3 -Wall -Wextra -pedantic
OBJ=nfa.o nfa_construct.o nfa_search.o nfa_syntax.o

all: nfa

nfa: $(OBJ) nfa.h
	$(CC) -o $@ $(OBJ) $(CFLAGS)

nfa.o: nfa.c nfa.h
	$(CC) -c $< $(CFLAGS)

nfa_construct.o: nfa_construct.c nfa.h
	$(CC) -c $< $(CFLAGS)

nfa_search.o: nfa_search.c nfa.h
	$(CC) -c $< $(CFLAGS)

nfa_syntax.o: nfa_syntax.c nfa.h
	$(CC) -c $< $(CFLAGS)

clean:
	rm -rf nfa $(OBJ)
