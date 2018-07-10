# degenerexp

So this is a pretty lousy regex engine and the only thing special about it is that it's *mine* (incidentally, I believe that's exactly what my
parents used to say about me, but I digress).

It has not been optimized at all, for the project's sole purpose was, from the outset, to attain
*enlightenment*--which, I am delighted to report, was achieved. This means I will no longer spend any more time on GitHub (and no, not because Microsoft bought GitHub ~~and it will ruin it like all the other things it bought~~)
for I shall be sitting on top of a distant mountain reveling in the sheer overwhelming beauty of life. And all it took to arrive
at this state was to write a shoddy regular expression engine!

I trust it goes without saying that it should not be used in production.

## Regex syntax

degenerexp may be somewhat, ah, *behind* its peers, but it still ~~deserves your love~~ supports a subset of modern regular
expression engines' syntax, which is the following:

- `?` matches the preceding item at most once;
- `*` matches the preceding item zero or more times;
- `+` matches the previous item one or more times;
- `|` two regular expressions may be joined by this infix operator and the resulting regular expression matches any string matching one of the expressions; 
- `()` parentheses may be used to deliniate regular expressions into a single item for the preceding operators to act on

E.g.: `(ab|c)*de?` is a valid regular expression that even degenerexp can handle ~~given enough support and love~~.

## How it works

It uses Dijkstra's shunting-yard algorithm modified to handle regular expressions which in turn uses Thompson's construction
to incrementally build a nondeterministic finite automaton (NFA). Then, subset construction is employed to turn the
NFA into a deterministic finite automaton (DFA). Finally, input strings are matched against this DFA.

```c++
const auto input = "abababde";
const auto regex = "(ab|c)*de";
fsm::nfa nfa = parser::shunting_yard_nfa_parser(regex).parse();
fsm::dfa dfa(nfa, fsm::derive_input_language(regex));
if(dfa.simulate(input_str) == fsm::result::accept) {
    std::cout << input << " matches the regex\'"" << regex << "'!\n";
}
```

It still needs a higher level wrapper that takes care of these steps, ~~but why bother once enlightenment has been reached?~~ so nominally it's still not finished.

## Resources

I used the following resources to build degenerexp:
- https://www.gamedev.net/articles/programming/general-and-gameplay-programming/finite-state-machines-and-regular-expressions-r3176
- https://en.wikipedia.org/wiki/Shunting-yard_algorithm
- https://en.wikipedia.org/wiki/Thompson%27s_construction
- http://www.cs.nuim.ie/~jpower/Courses/Previous/parsing/node9.html
- http://condor.depaul.edu/glancast/444class/docs/nfa2dfa.html
