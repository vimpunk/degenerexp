#ifndef THOMPSON_HEADER
#define THOMPSON_HEADER

#include "fsm.hpp"

namespace thompson {

inline fsm::nfa build_literal(const fsm::input_t input)
{
    fsm::nfa literal(2);
    literal.add_transition(0, 1, input);
    return literal;
}

inline fsm::nfa build_concatenation(const fsm::nfa& a, const fsm::nfa& b)
{
    auto tmp = a;
    tmp.chain(b);
    return tmp;
}

inline fsm::nfa build_alternation(const fsm::nfa& a, const fsm::nfa& b)
{
    fsm::nfa alt = a;
    alt.append(b);
    alt.prepend_empty_states(1);
    alt.append_empty_states(1);
    alt.add_transition(0, 1, fsm::epsilon);
    alt.add_transition(0, 1 + a.size(), fsm::epsilon);
    alt.add_transition(a.size(), alt.size() - 1, fsm::epsilon);
    alt.add_transition(a.size() + b.size(), alt.size() - 1, fsm::epsilon);
    return alt;
}

inline fsm::nfa build_kleene_star(const fsm::nfa& nfa)
{
    fsm::nfa star = nfa;
    star.prepend_empty_states(1);
    star.append_empty_states(1);
    star.add_transition(0, 1, fsm::epsilon);
    star.add_transition(0, star.size() - 1, fsm::epsilon);
    star.add_transition(star.size() - 2, 1, fsm::epsilon);
    star.add_transition(star.size() - 2, star.size() - 1, fsm::epsilon);
    return star;
}

inline fsm::nfa build_question_mark(const fsm::nfa& nfa)
{
    return build_alternation(nfa, build_literal(fsm::epsilon));
}

inline fsm::nfa build_plus_sign(const fsm::nfa& nfa)
{
    return build_concatenation(nfa, build_kleene_star(nfa));
}

} // thompson

#endif
