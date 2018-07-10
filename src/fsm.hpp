#ifndef NFA_HEADER
#define NFA_HEADER

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <stack>
#include <string_view>
#include <set>
#include <map>

namespace fsm {

using state_t = int;
using input_t = int;
enum {
    epsilon = -1,
};

enum class result {
    accept, reject
};

inline std::set<input_t> derive_input_language(std::string_view regex)
{
    return {regex.begin(), regex.end()};
}

struct nfa
{
    using transition_table_type = std::vector<std::vector<input_t>>;

private:
    transition_table_type transition_table_;
    std::set<input_t> input_language_;

public:
    explicit nfa(const int size, std::set<input_t> input_language = {})
        : transition_table_(size)
        , input_language_(std::move(input_language))
    {
        if(size < 1) {
            throw std::invalid_argument("n must be larger than zero");
        }
        assert(transition_table_.size() == size);
        for(auto& row : transition_table_) {
            row.resize(size);
        }
    }

    int size() const noexcept { return transition_table_.size(); }

    state_t start_state() const noexcept { return 0; }
    state_t final_state() const noexcept { return transition_table_.size() - 1; }

    const transition_table_type& transition_table() const noexcept
    {
        return transition_table_;
    }

    void add_transition(const state_t from, const state_t to, const input_t input)
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

    template<typename States = std::initializer_list<state_t>>
    std::set<state_t> epsilon_closure(const States& start_states) const
    {
        // algorithm eps-closure 
        //
        // inputs: N - NFA, T - set of NFA states 
        // output: eps-closure(T) - states reachable from T by eps transitions 
        //
        // eps-closure(T) = T foreach state t in T 
        //  push(t, stack) 
        //  while stack is not empty do 
        //  	t = pop(stack) 
        //      foreach state u with an eps edge from t to u 
        //      	if u is not in eps-closure(T) 
        //          	add u to eps-closure(T) 
        //              push(u, stack) 
        //          end 
        //  return eps-closure(T) 
        std::set<state_t> eps_closure;
        auto is_in_eps_closure = [&eps_closure](const state_t s) {
            return std::find(eps_closure.begin(), eps_closure.end(), s)
                != eps_closure.end();
        };
        for(auto s : start_states) {
            if(!is_legal_state(s)) {
                throw std::invalid_argument("invalid state");
            }
            std::stack<state_t> stack;
            stack.push(s);
            while(!stack.empty()) {
                const auto t = stack.top();
                stack.pop();
                for(int u = 0; u < size(); ++u) {
                    if((u == t || transition_table_[t][u] == epsilon)
                       && !is_in_eps_closure(u)) {
                        stack.push(u);
                        eps_closure.insert(u);
                    }
                }
            }
        }
        return eps_closure;
    }

    /**
     * Returns the states that are reachable from `start` given an input and
     * a single transition on this input (thus not considering intermediate
     * epsilon transitions).
     */
    std::set<state_t> reachable_states(const state_t start, const input_t input) const
    {
        return reachable_states({start}, input);
    }

    template<typename States = std::initializer_list<state_t>>
    std::set<state_t> reachable_states(const States& start_states, const input_t input) const
    {
        std::set<state_t> result;
        for(const auto start : start_states) {
            if(!is_legal_state(start)) {
                throw std::invalid_argument("invalid start state");
            }
            for(state_t s = 0; s < size(); ++s) {
                if(s == start) {
                    continue;
                }
                const auto i = transition_table_[start][s];
                if(i == input) {
                    result.insert(s);
                }
            }
        }
        return result;
    }

private:
    bool is_legal_state(const state_t s) const noexcept
    {
        return s >= 0 && s < transition_table_.size();
    }
};

struct dfa
{
    using transition_table_type =
        std::map<std::set<state_t>, std::map<input_t, std::set<state_t>>>;

private:
    transition_table_type state_map_;
    transition_table_type::const_iterator start_;
    state_t final_state_;

public:
    /** Constructs a DFA from an NFA and an input language via subset construction. */
    dfa(const nfa& nfa, const std::set<input_t>& input_lang)
        : start_(state_map_.end())
        , final_state_(nfa.final_state())
    {
        std::stack<std::set<state_t>> state_stack;
        state_stack.push(nfa.epsilon_closure({nfa.start_state()}));

        while(!state_stack.empty()) {
            auto start_states = std::move(state_stack.top());
            state_stack.pop();
            for(const auto input : input_lang) {
                // Compute all reachable states given `input`.
                const auto reachable = nfa.reachable_states(start_states, input);
                if(!reachable.empty()) {
                    // Compute the epsilon closure of `reachable` so that
                    // epsilon transitions are considered as well (the result
                    // includes the original `reachable` set).
                    const auto all_reachable = nfa.epsilon_closure(reachable);

                    // Connect the two states in the transition table.
                    auto& transitions = state_map_[start_states];
                    transitions[input] = all_reachable;
                    state_stack.push(std::move(all_reachable));
                    if(start_ == state_map_.end()) {
                        start_ = state_map_.begin();
                    }
                }
            }
        }

        assert(state_map_.find({final_state_}) == state_map_.end());
        state_map_[{final_state_}] = {};
    }

    transition_table_type transition_table() const { return state_map_; }

    result simulate(std::string_view input) const
    {
        auto it = start_;
        for(auto c : input) {
            if(it == state_map_.end()) { return result::reject; }
            const auto& [_, transitions] = *it;
            const auto& transition = transitions.find(c);
            //const auto transition = std::find_if(transitions.begin(), transitions.end(),
                    //[c](const auto& t) { return t.input == c; });
            if(transition == transitions.end()) { return result::reject; }
            it = state_map_.find(transition->second);
        }

        if(it != state_map_.end()) {
            const auto& [states, _] = *it;
            if(states.find(final_state_) != states.end()) {
                return result::accept;
            }
        }
        return result::reject;
    }
};

} // fsm

#endif
