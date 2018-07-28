#ifndef PARSER_HEADER
#define PARSER_HEADER

#include <string_view>
#include <stack>
#include <stdexcept>
#include <cassert>
#include <optional>

#include "fsm.hpp"
#include "thompson.hpp"

namespace parser {

enum class op
{
    alternation,
    question_mark,
    kleene_star,
    plus_sign,
    left_paren,
    right_paren,
};

char op_to_char(const op op) {
    switch(op) {
    case op::alternation: return '|';
    case op::kleene_star: return '*';
    case op::question_mark: return '?';
    case op::plus_sign: return '+';
    }
    return '-';
}

class shunting_yard_nfa_parser
{
    std::string_view regex_;
    std::stack<op> op_stack_;
    std::vector<fsm::nfa> output_;
    int nesting_level_ = 0;
    bool is_prev_separator_ = false;

public:
    explicit shunting_yard_nfa_parser(std::string_view regex) : regex_(regex) {}
    
    fsm::nfa parse()
    {
        // The regex has already been parsed, don't repeat the process.
        if(output_.size() == 1) {
            return output_.back();
        }

        for(const auto c : regex_) {
            switch(c) {
            case '(':
                op_stack_.push(op::left_paren);
                is_prev_separator_ = true;
                ++nesting_level_;
                break;
            case ')':
                while(!op_stack_.empty() && op_stack_.top() != op::left_paren) {
                    const auto op = op_stack_.top();
                    op_stack_.pop();
                    // The rest of the operators are evaluated in place.
                    assert(op == op::alternation);
                    build_alternation();
                }
                // Remove left paren.
                op_stack_.pop();
                is_prev_separator_ = true;
                --nesting_level_;
                break;
            case '*':
                build_kleene_star();
                is_prev_separator_ = false;
                // Since a multi (*,?,+) cannot follow another multi, this is the
                // right opportunity to concatenate the separate NFAs in the output
                // queue.
                concatenate_output();
                break;
            case '?':
                build_question_mark();
                is_prev_separator_ = false;
                // See comment in op::*.
                concatenate_output();
                break;
            case '+':
                build_plus_sign();
                is_prev_separator_ = false;
                // See comment in op::*.
                concatenate_output();
                break;
            case '|':
                op_stack_.push(op::alternation);
                is_prev_separator_ = true;
                break;
            default:
                auto literal = thompson::build_literal(c);
                if(is_prev_separator_) {
                    output_.emplace_back(std::move(literal));
                    is_prev_separator_ = false;
                } else {
                    if(output_.empty()) {
                        output_.emplace_back(literal);
                    } else {
                        output_.back() = thompson::build_concatenation(output_.back(), literal);
                    }
                }
            }
        }

        // Evaluate the remaining operators which at this point are only
        // binary operations (alternations).
        while(!op_stack_.empty()) {
            auto op = op_stack_.top();
            op_stack_.pop();
            assert(op == op::alternation);
            build_alternation();
        }

        assert(output_.size() == 1);
        return output_.front();
    }

private:
    void build_alternation()
    {
        if(output_.size() < 2) {
            throw std::runtime_error("| operator must have two arguments");
        }
        auto& first = output_[output_.size() - 2];
        auto& second = output_[output_.size() - 1];
        auto alt = thompson::build_alternation(first, second);
        output_.pop_back();
        // Save a memory allocation by moving the new NFA to existing memory.
        output_.back() = std::move(alt);
    }

    void concatenate_output()
    {
        // Concatenate individual NFAs.
        while(output_.size() > 1) {
            auto nfa = thompson::build_concatenation(output_[output_.size() - 2], output_[output_.size() - 1]);
            output_.pop_back();
            output_.back() = std::move(nfa);
        }
    }

    void build_kleene_star()
    {
        if(output_.empty()) {
            throw std::runtime_error("* operator must have an argument");
        }
        output_.back() = thompson::build_kleene_star(output_.back());
    }

    void build_question_mark()
    {
        if(output_.empty()) {
            throw std::runtime_error("? operator must have an argument");
        }
        output_.back() = thompson::build_question_mark(output_.back());
    }

    void build_plus_sign()
    {
        if(output_.empty()) {
            throw std::runtime_error("+ operator must have an argument");
        }
        output_.back() = thompson::build_plus_sign(output_.back());
    }
};

} // parser

#endif
