CC=gcc
CFLAGS=-g3 -Wall -Wextra -pedantic
OBJ=nfa.o nfa_construct.o nfa_search.o

all: nfa

nfa: nfa.o nfa_construct.o nfa_search.o nfa.h
	$(CC) -o $@ $(OBJ) $(CFLAGS)

nfa.o: nfa.c nfa.h
	$(CC) -c $< $(CFLAGS)

nfa_construct.o: nfa_construct.c nfa.h
	$(CC) -c $< $(CFLAGS)

nfa_search.o: nfa_search.c nfa.h
	$(CC) -c $< $(CFLAGS)

clean:
	rm -rf nfa $(OBJ)
