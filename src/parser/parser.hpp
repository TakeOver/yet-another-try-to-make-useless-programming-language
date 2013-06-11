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
                
        };
}
