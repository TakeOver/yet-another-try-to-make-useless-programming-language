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
                Statement,
                Identifer
        };
        struct ParseVal{
                ParseValType type;
                uint16_t token_id;
                std::wstring val;
                ParseVal(ParseValType t, std::wstring val): type(t), val(val){}
                ParseVal(ParseValType t):type(t){}
                static ParseVal Token(std::wstring val){
                        return ParseVal(ParseValType::Token, val);
                }
                static ParseVal Expr() {
                        return ParseVal(ParseValType::Expression);
                }
                static ParseVal Stmt(){
                        return ParseVal(ParseValType::Statement);
                }
                static ParseVal Ident(){
                        return ParseVal(ParseValType::Identifer);
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
                typedef  std::function<Statement*(std::vector<ParsedVal>&)>     stmt_alloc;
                typedef  std::function<Expression*(std::vector<ParsedVal>&)>    expr_alloc;
                typedef  std::function<Statement*(Parser&,Lexer&)>              stmt_parser;
                typedef  std::function<Expression*(Parser&,Lexer&)>             expr_parser;
                std::unordered_map<uint32_t, uint32_t>                          dispatch_by_token;
                std::vector<bool>                                               dispatch_types; // true - expression. false - stmt;
                std::vector<stmt_alloc >                                        stmt_allocators;
                std::vector<expr_alloc >                                        expr_allocators; 
                std::vector<std::vector<ParseVal>>                              rules; 
                std::vector<bool>                                               is_rule; // is rule or  user function .
                std::vector<stmt_parser>                                        stmt_parsers;
                std::vector<expr_parser>                                        expr_parsers;
                
                Lexer lex;
                
                bool            failed = false;
                std::wstring    error;
                
                void HandleError(std::wstring msg, tok_info_t ti){
                        failed = true;
                        lex.HandleError(msg,ti);
                        error = lex.error;
                        DBG_TRACE();
                }
                
                void ForceLexerError(){
                        failed = lex.failed;
                        error = lex.error;
                }
                
                uint32_t dispatch(token_t & tok){
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
                
                std::vector<ParsedVal> processRule(uint32_t dp, token_t & tok){                        
                        std::vector<ParsedVal> parsed;
                        auto& rule = rules[dp];
                        parsed.push_back({rule.front()});
                        DBG_TRACE("rule:%d",dp);
                        for(std::vector<ParseVal>::iterator i = rule.begin()+1, e = rule.end(); i!=e;++i){
                                DBG_TRACE("%d %d",i->type, i->token_id);
                                if(i->type == ParseValType::Token){
                                        auto token = lex.nextTok();
                                        if(token.id != i->token_id){
                                                HandleError(L"Expected: " + i->val + L", found:"+ token.val, token.tokinfo);
                                                finallize(parsed);
                                                return std::vector<ParsedVal>();
                                        }
                                        parsed.push_back(ParsedVal(*i));
                                        continue;
                                }
                                if(i->type == ParseValType::Identifer){
                                        auto id = lex.nextTok();
                                        if(id.tok!=Token::IDENTIFER && id.id!=static_cast<int>(id.tok)){
                                                HandleError(L"Identifer expected, found:" + id.val,id.tokinfo);
                                                finallize(parsed);
                                                return std::vector<ParsedVal>();
                                        }
                                        parsed.push_back(ParsedVal(*i));
                                        continue;
                                }
                                if(i->type == ParseValType::Expression){
                                        auto expr = expectExpression();
                                        if(!expr){
                                                finallize(parsed);
                                                return std::vector<ParsedVal>();
                                        }
                                        parsed.push_back({*i,expr});
                                        continue;
                                }
                                assert(i->type == ParseValType::Statement && "statement");
                                auto stmt = expectStatement();
                                if(!stmt){
                                        finallize(parsed);
                                        return std::vector<ParsedVal>();
                                }
                                parsed.push_back({*i,stmt});
                        }
                        return parsed;
                }
                
                Statement* statement_factory(uint32_t dp, std::vector<ParsedVal>& parsed){
                        return stmt_allocators[dp](parsed);
                }
                
                Expression* expression_factory(uint32_t dp, std::vector<ParsedVal>& parsed){
                        return expr_allocators[dp](parsed);
                }
                
                void defRule(ParseValType pvt, std::vector<ParseVal> rule){
                        expr_parsers.push_back(nullptr);
                        stmt_parsers.push_back(nullptr);
                        is_rule.push_back(true);
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
                        dispatch_by_token[id] = rules.size();
                        rules.push_back(rule);
                }

        public:
                Parser(){
                        rules.push_back({});
                        expr_allocators.push_back(nullptr);
                        stmt_allocators.push_back(nullptr);
                        stmt_parsers.push_back(nullptr);
                        expr_parsers.push_back(nullptr);
                        dispatch_types.push_back(false);
                        is_rule.push_back(true);
                }
                
                Statement* expectStatement(bool move = true){
                        DBG_TRACE();
                        ForceLexerError();
                        if(failed)
                                return nullptr;
                        token_t tok (Token::NONE, {0,0},L"");
                        if(move)
                                tok = lex.nextTok();
                        else 
                                tok = lex.lastTok();
                        auto dp = dispatch(tok);
                        if(!dp || dispatch_types[dp])
                                return new Statement(expectExpression(false)); // hack. I know. ;)
                        if(!is_rule[dp])
                                return stmt_parsers[dp](*this,lex);
                        auto parsed = processRule(dp,tok);
                        if(parsed.size()==0)
                                return nullptr;
                        return statement_factory(dp,parsed);
                }
                
                Expression * expectExpression(bool move = true){
                        DBG_TRACE();
                        ForceLexerError();
                        if(failed)
                                return nullptr;
                        auto tok = move?lex.nextTok():lex.lastTok();
                        auto dp = dispatch(tok);
                        if(!dp || dispatch_types[dp]==false){
                                //TODO EXPR PARSING.
                                return new Expression();
                        }
                        if(!is_rule[dp])
                                return expr_parsers[dp](*this,lex);
                        auto parsed = processRule(dp,tok);
                        if(parsed.size()==0)
                                return nullptr;
                        return expression_factory(dp,parsed);
                }
                
                void addData(std::wstring data, std::wstring file){
                        lex.addData(data,file);
                }
                
                void Parse(){
                        delete expectExpression(); // lol. just testing.
                }
                
                void showError(){
                        std::wcerr << L"error:" << error << std::endl;
                }
                
                void defStmt(std::vector<ParseVal> rule,stmt_alloc alloc){
                        expr_allocators.push_back(nullptr);
                        stmt_allocators.push_back(alloc);
                        dispatch_types.push_back(false);
                        defRule(lambda::ParseValType::Statement, rule);
                }
                
                void defExpr(std::vector<ParseVal> rule,expr_alloc alloc){
                        stmt_allocators.push_back(nullptr);
                        expr_allocators.push_back(alloc);
                        dispatch_types.push_back(true);
                        defRule(lambda::ParseValType::Expression, rule);
                }
                
                void defExpr(ParseVal begin_tok,expr_parser allocator){
                        assert(begin_tok.type == ParseValType::Token&& "defExpr User Function, begin_tok!=Token");
                        is_rule.push_back(false);
                        expr_parsers.push_back(allocator);
                        stmt_parsers.push_back(nullptr);
                        dispatch_by_token[lex.TryRecognize(begin_tok.val)] = rules.size(); // dirty hack; :C
                        rules.push_back({});
                }
                
                void defStmt(ParseVal begin_tok,stmt_parser allocator){
                        assert(begin_tok.type == ParseValType::Token&& "defExpr User Function, begin_tok!=Token");
                        is_rule.push_back(false);
                        stmt_parsers.push_back(allocator);
                        expr_parsers.push_back(nullptr);
                        dispatch_by_token[lex.TryRecognize(begin_tok.val)] = rules.size();
                        rules.push_back({});
                }
                
                void showRules(){
                        for(auto&x:dispatch_by_token){
                                std::wcerr << x.first << L'\t';
                                if(!is_rule[x.second]){
                                        std::wcerr << lex.tokById(x.first) << L"\t user-spec. parser function\n";
                                        continue;
                                }
                                for(auto&x:rules[x.second]){
                                        if(x.type == ParseValType::Token)
                                                std::wcerr << x.val << L"(" << x.token_id << L")\t";
                                        else if(x.type == ParseValType::Expression)
                                                std::wcerr << L"$expression\t";
                                        else if(x.type == ParseValType::Statement)
                                                std::wcerr << L"#statement\t";
                                        else
                                                std::wcerr << L"%identifer\t";
                                } 
                                std::wcerr << std::endl;
                        }
                }
        };
}
