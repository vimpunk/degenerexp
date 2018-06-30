#include <iostream>
#include <iomanip>
#include <cassert>

#include "../src/fsm.hpp"

std::ostream& operator<<(std::ostream& out, const fsm::nfa& nfa)
{
    for(const auto& row : nfa.transition_table()) {
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

int main()
{
    nfa();
}
