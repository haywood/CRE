#!/bin/sh

./nfa 'a' 'abcabcabc'
./nfa 'a*' 'abcabcabc'
./nfa 'a*?' 'abcabcabc'
./nfa 'a+' 'abcabcabc'
./nfa 'a+?' 'abcabcabc'
./nfa 'a?' 'abcabcabc'
./nfa 'a??' 'abcabcabc'

./nfa 'ab' 'abcabcabc'
./nfa '(ab)*' 'abcabcabc'
./nfa '(ab)*?' 'abcabcabc'
./nfa '(ab)+' 'abcabcabc'
./nfa '(ab)+?' 'abcabcabc'
./nfa '(ab)?' 'abcabcabc'
./nfa '(ab)??' 'abcabcabc'

./nfa 'abc' 'abcabcabc'
./nfa '(abc)*' 'abcabcabc'
./nfa '(abc)*?' 'abcabcabc'
./nfa '(abc)+' 'abcabcabc'
./nfa '(abc)+?' 'abcabcabc'
./nfa '(abc)?' 'abcabcabc'
./nfa '(abc)??' 'abcabcabc'

./nfa 'ab*' 'abcabcabc'
./nfa 'ab*?' 'abcabcabc'