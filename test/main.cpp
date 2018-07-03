#include <iostream>
#include <iomanip>
#include <cassert>

#include "../src/fsm.hpp"
#include "../src/thompson.hpp"
#include "../src/parser.hpp"

std::ostream& operator<<(std::ostream& out, const std::vector<std::vector<fsm::state>>& table)
{
    for(const auto& row : table) {
        for(const auto& s : row) {
            out << std::setw(2);
            if(s == 0) {
                out << "00" << ' ';
            } else {
                out << s << ' ';
            }
        }
        out << '\n';
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const fsm::nfa& nfa)
{
    return out << nfa.transition_table();
}

void nfa()
{
    fsm::nfa nfa1(2);
    nfa1.add_transition(0, 1, 1);
    nfa1.add_transition(1, 0, 2);
    std::cout << nfa1 << '\n';
    const auto nfa1_expected = fsm::transition_table_type{
        {0,1},
        {2,0},
    };
    assert(nfa1.transition_table() == nfa1_expected);

    auto nfa1_2 = nfa1;
    nfa1_2.prepend_empty_states(1);
    std::cout << nfa1_2 << '\n';
    const auto nfa1_2_expected = fsm::transition_table_type{
        {0,0,0},
        {0,0,1},
        {0,2,0},
    };
    assert(nfa1_2.transition_table() == nfa1_2_expected);

    auto nfa1_3 = nfa1_2;
    nfa1_3.append_empty_states(2);
    std::cout << nfa1_3 << '\n';
    const auto nfa1_3_expected = fsm::transition_table_type{
        {0,0,0,0,0},
        {0,0,1,0,0},
        {0,2,0,0,0},
        {0,0,0,0,0},
        {0,0,0,0,0},
    };
    assert(nfa1_3.transition_table() == nfa1_3_expected);

    fsm::nfa nfa2(3);
    nfa2.add_transition(0, 1, 3);
    nfa2.add_transition(1, 0, 4);
    nfa2.add_transition(2, 1, 5);
    std::cout << nfa2 << '\n';
    const auto nfa2_expected = fsm::transition_table_type{
        {0,3,0},
        {4,0,0},
        {0,5,0},
    };
    assert(nfa2.transition_table() == nfa2_expected);

    auto nfa3 = nfa1;
    nfa3.prepend(nfa2);
    std::cout << nfa3 << '\n';
    const auto nfa3_expected = fsm::transition_table_type{
        {0,3,0,0,0},
        {4,0,0,0,0},
        {0,5,0,0,0},
        {0,0,0,0,1},
        {0,0,0,2,0},
    };
    assert(nfa3.transition_table() == nfa3_expected);

    auto nfa4 = nfa2;
    nfa4.chain(nfa1);
    std::cout << nfa4 << '\n';
    const auto nfa4_expected = fsm::transition_table_type{
        {0,3,0,0},
        {4,0,0,0},
        {0,5,0,1},
        {0,0,2,0},
    };
    assert(nfa4.transition_table() == nfa4_expected);
}

void thompson_construction()
{
    auto a = thompson::build_literal('a');
    std::cout << "'a':\n" << a << '\n';
    const auto a_expected = fsm::transition_table_type{
        {0,'a'},
        {0,0},
    };
    assert(a.transition_table() == a_expected);

    auto b = thompson::build_literal('b');
    std::cout << "'b':\n" << b << '\n';
    const auto b_expected = fsm::transition_table_type{
        {0,'b'},
        {0,0},
    };
    assert(b.transition_table() == b_expected);

    auto ab = thompson::build_concatenation(a, b);
    std::cout << "ab:\n" << ab << '\n';
    const auto ab_expected = fsm::transition_table_type{
        {0,'a',0},
        {0,0,'b'},
        {0,0,0},
    };
    assert(ab.transition_table() == ab_expected);


    auto a_or_b = thompson::build_alternation(a, b);
    std::cout << "a|b:\n" << a_or_b << '\n';
    const auto a_or_b_expected = fsm::transition_table_type{
        {0,fsm::epsilon,0,fsm::epsilon,0,0},
        {0,0,'a',0,0,0},
        {0,0,0,0,0,fsm::epsilon},
        {0,0,0,0,'b',0},
        {0,0,0,0,0,fsm::epsilon},
        {0,0,0,0,0,0},
    };
    assert(a_or_b.transition_table() == a_or_b_expected);

    fsm::nfa a_star = thompson::build_kleene_star(a);
    std::cout << "a*:\n" << a_star << '\n';
    const auto a_star_expected = fsm::transition_table_type{
        {0,fsm::epsilon,0,fsm::epsilon},
        {0,0,'a',0},
        {0,fsm::epsilon,0,fsm::epsilon},
        {0,0,0,0},
    };
    assert(a_star.transition_table() == a_star_expected);

    auto zero_or_one_a = thompson::build_question_mark(a);
    std::cout << "a?:\n" << zero_or_one_a << '\n';
    const auto zero_or_one_a_expected = fsm::transition_table_type{
        {0,fsm::epsilon,0,fsm::epsilon,0,0},
        {0,0,'a',0,0,0},
        {0,0,0,0,0,fsm::epsilon},
        {0,0,0,0,fsm::epsilon,0},
        {0,0,0,0,0,fsm::epsilon},
        {0,0,0,0,0,0},
    };
    assert(zero_or_one_a.transition_table() == zero_or_one_a_expected);

    auto a_or_b_star = thompson::build_kleene_star(a_or_b);
    std::cout << "(a|b)*:\n" << a_or_b_star << '\n';
    const auto a_or_b_star_expected = fsm::transition_table_type{
        {0,fsm::epsilon,0,0,0,0,0,fsm::epsilon},
        {0,0,fsm::epsilon,0,fsm::epsilon,0,0,0},
        {0,0,0,'a',0,0,0,0},
        {0,0,0,0,0,0,fsm::epsilon,0},
        {0,0,0,0,0,'b',0,0},
        {0,0,0,0,0,0,fsm::epsilon,0},
        {0,fsm::epsilon,0,0,0,0,0,fsm::epsilon},
        {0,0,0,0,0,0,0,0},
    };
    assert(a_or_b_star.transition_table() == a_or_b_star_expected);

    auto a_or_b_ab = thompson::build_concatenation(a_or_b, thompson::build_concatenation(a, b));
    std::cout << "(a|b)ab:\n" << a_or_b_ab << '\n';
}

void parse()
{
    auto regex1 = "(a|b)*cde";
    auto nfa1 = parser::shunting_yard_nfa_parser(regex1).parse();
    const auto expected1_hand_built = fsm::transition_table_type{
        {0,fsm::epsilon,0,0,0,0,0,fsm::epsilon,0,0,0},
        {0,0,fsm::epsilon,0,fsm::epsilon,0,0,0,0,0,0},
        {0,0,0,'a',0,0,0,0,0,0,0},
        {0,0,0,0,0,0,fsm::epsilon,0,0,0,0},
        {0,0,0,0,0,'b',0,0,0,0,0},
        {0,0,0,0,0,0,fsm::epsilon,0,0,0,0},
        {0,fsm::epsilon,0,0,0,0,0,fsm::epsilon,0,0,0},
        {0,0,0,0,0,0,0,0,'c',0,0},
        {0,0,0,0,0,0,0,0,0,'d',0},
        {0,0,0,0,0,0,0,0,0,0,'e'},
        {0,0,0,0,0,0,0,0,0,0,0},
    };
    const auto expected1 = [] {
        // Since Thompson construction has been tested before, assemble the
        // larger NFA peacemeal with Thompson primitives.
        const auto a = thompson::build_literal('a');
        const auto b = thompson::build_literal('b');
        const auto a_or_b = thompson::build_alternation(a, b);
        const auto a_or_b_star = thompson::build_kleene_star(a_or_b);
        const auto c = thompson::build_literal('c');
        const auto d = thompson::build_literal('d');
        const auto e = thompson::build_literal('e');
        const auto cd = thompson::build_concatenation(c, d);
        const auto cde = thompson::build_concatenation(cd, e);
        const auto a_or_b_star_cde = thompson::build_concatenation(a_or_b_star, cde);
        return a_or_b_star_cde.transition_table();
    }();
    std::cout << "(a|b)*cde:\n" << nfa1 << '\n';
    assert(expected1_hand_built == expected1);
    assert(nfa1.transition_table() == expected1);

    auto regex2 = "a(b|c)*|d";
    auto nfa2 = parser::shunting_yard_nfa_parser(regex2).parse();
    const auto expected2 = [] {
        // Since Thompson construction has been tested before, assemble the
        // larger NFA peacemeal with Thompson primitives.
        const auto b = thompson::build_literal('b');
        const auto c = thompson::build_literal('c');
        const auto b_or_c = thompson::build_alternation(b, c);
        const auto b_or_c_star = thompson::build_kleene_star(b_or_c);
        const auto a = thompson::build_literal('a');
        const auto a_b_or_c_star = thompson::build_concatenation(a, b_or_c_star);
        const auto d = thompson::build_literal('d');
        const auto a_and_b_or_c_star_or_d = thompson::build_alternation(a_b_or_c_star, d);
        return a_and_b_or_c_star_or_d.transition_table();
    }();
    std::cout << "a(b|c)*|d:\n" << nfa2 << '\n';
    assert(nfa2.transition_table() == expected2);
}

void eps_closure()
{
    const fsm::nfa nfa = parser::shunting_yard_nfa_parser("(a|b)*abb").parse();
    const auto eps_closure = nfa.epsilon_closure();
    assert(eps_closure == std::vector<fsm::state>{
            // TODO
            });
}

int main()
{
    nfa();
    thompson_construction();
    parse();
}
