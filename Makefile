CC=gcc
CFLAGS=-g3 -O0 -Wall -Wextra -pedantic
OBJ=nfa.o buildNFA.o nfa_search.o nfa_syntax.o MatchObject.o

all: nfa

nfa: $(OBJ) nfa.h re.h
	$(CC) -o $@ $(OBJ) $(CFLAGS)

nfa.o: nfa.c re.h
	$(CC) -c $< $(CFLAGS)

buildNFA.o: buildNFA.c nfa.h
	$(CC) -c $< $(CFLAGS)

nfa_search.o: nfa_search.c nfa.h
	$(CC) -c $< $(CFLAGS)

nfa_syntax.o: nfa_syntax.c nfa.h
	$(CC) -c $< $(CFLAGS)

MatchObject.o: MatchObject.c nfa.h
	$(CC) -c $< $(CFLAGS)

clean:
	rm -rf nfa $(OBJ)
