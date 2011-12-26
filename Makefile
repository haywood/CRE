CC=gcc
CFLAGS=-g3 -O0 -Wall -Wextra -pedantic
INC=re.h searchNFA.h MatchObject.h buildMYT.h checkRE.h State.h SymbolVector.h

all: nfa

nfa: nfa.c $(INC)
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm -rf nfa
