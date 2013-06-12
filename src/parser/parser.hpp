#pragma once
#include "../lexer/lexer.hpp"
#include <map>
#include <functional>
#include <exception>
#include "../ast/statement.hpp"
#include "../ast/expression.hpp"
#include "../utils/debug.hpp"
namespace lambda{
        enum class ParseValType{
                Token = 0,
                Expression,
                Statement
        };
        struct ParseVal{
                ParseValType type;
                uint16_t token_id;
                std::wstring val;
                ParseVal(ParseValType t, std::wstring val): type(t), val(val){}
                ParseVal(ParseValType t):type(t){}
                static ParseVal defToken(std::wstring val){
                        return ParseVal(ParseValType::Token, val);
                }
                static ParseVal Expr(){
                        return ParseVal(ParseValType::Expression);
                }
                static ParseVal Stmt(){
                        return ParseVal(ParseValType::Statement);
                }

        };
        struct ParsedVal{
                ParseVal pv;                
                union{
                        Expression * expr;
                        Statement * stmt;
                };

                ParsedVal(ParseVal pv, Statement*stmt): pv(pv), stmt(stmt){}
                ParsedVal(ParseVal pv, Expression*expr): pv(pv), expr(expr){}
                ParsedVal(ParseVal pv): pv(pv),expr(nullptr){}
        };
        class Parser{
                std::unordered_map<uint32_t, uint32_t>  dispatch_by_token;
                std::vector<std::vector<ParseVal>> movs;
                Lexer lex;
                bool failed = false;
                std::wstring error;
                void HandleError(std::wstring msg, tok_info_t ti){
                        failed = true;
                        lex.HandleError(msg,ti);
                        error = lex.error;
                        DBG_TRACE();
                }
                Expression * expectExpression(){
                        DBG_TRACE();
                        lex.nextTok(); // заглушка. для тестов.
                        return new Expression(); //TODO
                }
                uint32_t dispatch(token_t & tok){
                        DBG_TRACE();
                        auto iter = dispatch_by_token.find(tok.id);
                        if(iter == dispatch_by_token.end())
                                return 0;
                        return iter->second;
                }
                static void finallize(std::vector<ParsedVal> vec){
                        for(auto&x:vec)
                                if(x.pv.type == ParseValType::Expression)
                                        delete x.expr;
                                else if(x.pv.type == ParseValType::Statement)
                                        delete x.stmt;
                }
                Statement* expectStatement(){

                        auto tok = lex.nextTok();
                        auto dp = dispatch(tok);
                        if(!dp)
                                return new Statement(expectExpression()); // hack. I know. ;)
                        DBG_TRACE("dispatch:%d",dp);
                        std::vector<ParsedVal> parsed;
                        auto& rule = movs[dp];
                        for(std::vector<ParseVal>::iterator i = rule.begin()+1, e = rule.end(); i!=e;++i){
                                DBG_TRACE("%d %d",i->type, i->token_id);
                                if(i->type == ParseValType::Token){
                                        auto tok = lex.nextTok();\
                                        if(tok.id != i->token_id){
                                                HandleError(L"Expected: " + i->val + L", found:"+ tok.val, tok.tokinfo);
                                                finallize(parsed);
                                                return nullptr;
                                        }
                                        parsed.push_back(ParsedVal(*i));
                                        continue;
                                }
                                if(i->type == ParseValType::Expression){
                                        auto expr = expectExpression();
                                        if(!expr){
                                                finallize(parsed);
                                                return nullptr;
                                        }
                                        parsed.push_back({*i,expr});
                                        continue;
                                }
                                assert(i->type == ParseValType::Statement && "statement");
                                auto stmt = expectStatement();
                                if(!stmt){
                                        finallize(parsed);
                                        return nullptr;
                                }
                                parsed.push_back({*i,stmt});
                        }
                        return statement_factory(dp,parsed);
                }
                Statement* statement_factory(uint32_t dp, std::vector<ParsedVal>& parsed){
                        //TODO
                        return new Statement();
                }

        public:
                Parser(){
                        movs.push_back({});
                }
                void addData(std::wstring data, std::wstring file){
                        lex.addData(data,file);
                }
                void Parse(){
                        delete expectStatement(); // lol. just testing.
                }
                void showError(){
                        std::wcerr << L"error:" << error << std::endl;
                }
                void defRule(std::vector<ParseVal> rule){
                        if(rule.size()==0) // FIXME
                                throw  std::string("Rule size  == 0");
                        if(rule.front().type!=ParseValType::Token)
                                throw std::string("First element of rule != Token");
                        for(auto&x:rule){
                                if(x.type == ParseValType::Token){
                                        x.token_id = lex.TryRecognize(x.val);
                                }
                        }
                        auto id = rule.front().token_id;
                        dispatch_by_token[id] = movs.size();
                        movs.push_back(rule);
                }
                void showRules(){
                        for(auto&x:dispatch_by_token){
                                std::wcerr << x.first << L'\t';
                                for(auto&x:movs[x.second]){
                                        if(x.type == ParseValType::Token)
                                                std::wcerr << x.val << L'\t';
                                        else if(x.type == ParseValType::Expression)
                                                std::wcerr << L"$expression\t";
                                        else
                                                std::wcerr << L"#statement\t";
                                } 
                        }
                        std::wcerr << std::endl;
                }
        };
}
