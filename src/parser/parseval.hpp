#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../lexer/token.hpp"
#include "../ast/statement.hpp"
#include "../ast/expression.hpp"

namespace lambda{
        enum class ParseValType{
                Token = 0,
                Expression,
                Statement,
                Identifer,
                Rule,
                String,
                Operator,
                Number,
                Term,
                SelfRecursive // this would be preprocessed via defRule into Rule(Parser::rules_count());
        };
        
        struct ParseVal{
                
                ParseValType type;
                uint16_t token_id = 0;
                uint32_t rule_id = 0;
                bool any = false; // i want to reduce 'inf*' parse-types. and make code less ugly
                std::wstring val; // i'll put String/Number/Operator in val.

                ParseVal(ParseValType t, std::wstring val): type(t), val(val){}
                ParseVal(ParseValType t):type(t){}
                ParseVal(ParseValType t, uint32_t id):type(t), rule_id(id){}
                inline static ParseVal Token(std::wstring val){
                        return ParseVal(ParseValType::Token, val);
                }
                inline static ParseVal Expr() {
                        return ParseVal(ParseValType::Expression);
                }
                inline static ParseVal Stmt(){
                        return ParseVal(ParseValType::Statement);
                }
                inline static ParseVal Any(ParseVal what){
                        what.any = true;
                        return what;
                }
                inline static ParseVal Ident(){
                        return ParseVal(ParseValType::Identifer);
                }
                inline static ParseVal Rule(uint32_t id){
                        return ParseVal(ParseValType::Rule,id);
                }
                inline static ParseVal Str(){
                        return ParseVal(ParseValType::String);
                }
                inline static ParseVal Oper(){
                        return ParseVal(ParseValType::Operator);
                }
                inline static ParseVal Num(){
                        return ParseVal(ParseValType::Number);
                }
                inline static ParseVal Term(){
                        return ParseVal(ParseValType::Term);
                }
                inline static ParseVal SelfRecursive(){
                        return ParseVal(ParseValType::SelfRecursive);
                }
                static const std::wstring type2wstr(bool any,ParseValType type){
                        using i = ParseValType;
                        const std::wstring prefix = (any)?L"any!":L""; 
                        switch (type){
                                case i::Token: return prefix+L"tok";
                                case i::Expression: return prefix+L"expr";
                                case i::Statement: return prefix+L"stmt";
                                case i::Term: return prefix+L"term";
                                case i::Rule: return prefix+L"rule";
                                case i::Identifer: return prefix+L"id";
                                case i::Number: return prefix+L"num";
                                case i::Operator: return prefix+L"op";
                                case i::String: return prefix+L"str";
                                default: return L"$err"; 
                        }
                }
                static const std::string type2str(bool any,ParseValType type){
                        using i = ParseValType;
                        const std::string prefix = (any)?"any!":""; 
                        switch (type){
                                case i::Token: return prefix+"tok";
                                case i::Expression: return prefix+"expr";
                                case i::Statement: return prefix+"stmt";
                                case i::Term: return prefix+"term";
                                case i::Rule: return prefix+"rule";
                                case i::Identifer: return prefix+"id";
                                case i::Number: return prefix+"num";
                                case i::Operator: return prefix+"op";
                                case i::String: return prefix+"str";
                                default: return "$err"; 
                        }
                }

        };
        struct ParsedVal{
                ParseVal pv;     

                union{
                        Statement  ** stmt = nullptr;
                        Expression ** expr;
                };
                uint32_t size = 0;

                std::vector<token_t> vals;

                ParsedVal(ParseVal pv, Statement**stmt, uint32_t size = 1): pv(pv), stmt(stmt), size(size){}
                ParsedVal(ParseVal pv, Expression**expr, uint32_t size = 1): pv(pv), expr(expr), size(size){}
                ParsedVal(ParseVal pv): pv(pv),expr(nullptr){}
                ParsedVal(ParseVal pv,std::vector<token_t> && vals):pv(pv), vals(vals){}
                ParsedVal(ParseVal pv,std::vector<token_t> vals):pv(pv), vals(vals){}
        };
}
