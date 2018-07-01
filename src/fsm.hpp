#ifndef NFA_HEADER
#define NFA_HEADER

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cassert>

namespace fsm {

using state = int;
using input = int;
using transition_table_type = std::vector<std::vector<input>>;
enum {
    epsilon = -1,
};


class nfa
{
    transition_table_type transition_table_;

public:
    explicit nfa(const int size) : transition_table_(size)
    {
        if(size < 1) {
            throw std::invalid_argument("n must be larger than zero");
        }
        assert(transition_table_.size() == size);
        for(auto& row : transition_table_) {
            row.resize(size);
        }
    }

    int size() const noexcept
    {
        return transition_table_.size();
    }

    const transition_table_type& transition_table() const noexcept
    {
        return transition_table_;
    }

    void add_transition(const state from, const state to, const input input)
    {
        if(!is_legal_state(from) || !is_legal_state(to)) {
            throw std::invalid_argument("invalid state");
        }
        transition_table_[from][to] = input;
    }

    /** Extends this NFA's end by n empty states. */
    void append_empty_states(const int n)
    {
        if(n < 1) {
            throw std::invalid_argument("n must be larger than zero");
        }
        const auto new_size = size() + n;
        transition_table_.resize(new_size);
        for(auto& row : transition_table_) {
            row.resize(new_size);
        }
    }

    /** Extends this NFA's start by n empty states. */
    void prepend_empty_states(const int n)
    {
        if(n < 1) {
            throw std::invalid_argument("n must be larger than zero");
        }
        const auto new_size = size() + n;
        transition_table_.resize(new_size);
        for(auto& row : transition_table_) {
            row.resize(new_size);
            // Shift row elements to the right by n.
            std::copy_backward(row.begin(), row.end() - n, row.end());
            std::fill(row.begin(), row.begin() + n, 0);
        }

        // Shift rows down by n.
        std::copy_backward(transition_table_.begin(), transition_table_.end() - n, transition_table_.end());
        for(auto it = transition_table_.begin(), end = it + n; it != end; ++it) {
            std::fill(it->begin(), it->end(), 0);
        }
    }

    /**
     * Places the other NFA before this NFA such that the other NFA's final
     * state is kept intact.
     */
    void prepend(const nfa& other)
    {
        if(this == &other) { return; }
        prepend_empty_states(other.size());
        for(auto i = 0; i < other.size(); ++i) {
            auto& this_row = transition_table_[i];
            const auto& other_row = other.transition_table_[i];
            std::copy(other_row.begin(), other_row.end(), this_row.begin());
        }
    }

    /**
     * Places the other NFA after this NFA such that this NFA's final state
     * is kept intact.
     */
    void append(const nfa& other)
    {
        if(this == &other) { return; }
        const int orig_size = size();
        append_empty_states(other.size());
        for(auto this_i = orig_size, other_i = 0; this_i < size(); ++this_i, ++other_i) {
            auto& this_row = transition_table_[this_i];
            const auto& other_row = other.transition_table_[other_i];
            std::copy(other_row.begin(), other_row.end(), this_row.begin() + orig_size);
        }
    }

    /**
     * Chains the other NFA such that this NFA's final state is merged with
     * the other NFA's start state.
     */
    void chain(const nfa& other)
    {
        if(this == &other) { return; }
        const int orig_size = size();
        // Subtract one from the resulting size becaues this NFA's final state
        // is going to be removed.
        append_empty_states(other.size() - 1);
        for(auto this_i = orig_size - 1, other_i = 0; this_i < size(); ++this_i, ++other_i) {
            assert(this_i < size());
            assert(other_i < other.size());
            auto& this_row = transition_table_[this_i];
            const auto& other_row = other.transition_table_[other_i];
            assert(this_row.begin() + orig_size - 1 < this_row.end());
            std::copy(other_row.begin(), other_row.end(), this_row.begin() + orig_size - 1);
        }
    }

private:
    bool is_legal_state(const state s) 
    {
        return s >= 0 && s < transition_table_.size();
    }
};

} // fsm

#endif
