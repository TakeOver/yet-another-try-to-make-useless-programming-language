#pragma once
#include "../lexer/lexer.hpp"
#include <map>
#include <functional>

#include "../ast/statement.hpp"
#include "../ast/expression.hpp"

namespace lambda{
        class Parser{
                std::unordered_map<
                                        std::wstring,
                                        std::function<Expression*(Parser*,Lexer*)>
                                  > expressions;
                std::unordered_map<
                                        std::wstring,
                                        std::function<Statement*(Parser*,Lexer*)>
                                  > statements;
                // algo - parser see's token and finding by dispatch current move, next move and parser creates stack based DFA and prsing text. LL(1), possibly to make LL(k)
                std::vector<uint32_t>  dispath_by_token;
                std::vector<uint32_t>  next_move;
                std::vector<std::function<Expression*(Parser*)>> allocators;
        };
}
