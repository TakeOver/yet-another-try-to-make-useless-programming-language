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
                Identifer,
                Rule
        };
        struct ParseVal{
                
                ParseValType type;
                uint16_t token_id = 0;
                uint32_t rule_id = 0;
                std::wstring val;

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
                inline static ParseVal Ident(){
                        return ParseVal(ParseValType::Identifer);
                }
                inline static ParseVal Rule(uint32_t id){
                        return ParseVal(ParseValType::Rule,id);
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
                std::unordered_map<std::wstring, uint32_t>                      rules_name;
                Lexer lex;
                
                bool            failed = false;
                std::wstring    error;
                
                inline void HandleError(std::wstring msg, tok_info_t ti){
                        failed = true;
                        lex.HandleError(msg,ti);
                        error = lex.error;
                        DBG_TRACE();
                }
                
                inline void ForceLexerError(){
                        failed = lex.failed;
                        error = lex.error;
                }
                
                inline uint32_t dispatch(token_t & tok){
                        auto iter = dispatch_by_token.find(tok.id);
                        if(iter == dispatch_by_token.end())
                                return 0;
                        return iter->second;
                }
                
                static inline void finallize(std::vector<ParsedVal> vec){
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
                                if(i->type == ParseValType::Rule){
                                        auto id = i->rule_id;
                                        token_t _tok = lex.nextTok();
                                        if(!id){
                                                HandleError(L"Rule expected(id == 0)", _tok.tokinfo);
                                                finallize(parsed);
                                                return std::vector<ParsedVal>();
                                        }
                                        ParseVal rule_begin = rules[id].front();
                                        if(rule_begin.token_id != _tok.id){
                                                HandleError(L"Expected rule with token:"+ rule_begin.val + L" found:"+ _tok.val, _tok.tokinfo);
                                                finallize(parsed);
                                                return std::vector<ParsedVal>();
                                        }
                                        auto processed = processRule(id, _tok);
                                        if(processed.size() == 0){
                                                finallize(parsed);
                                                return std::vector<ParsedVal>();
                                        }
                                        auto type = dispatch_types[id];

                                        if(type){
                                                auto expr = expression_factory(id, processed);
                                                if(!expr){
                                                        finallize(parsed);
                                                        return std::vector<ParsedVal>();
                                                }
                                                parsed.push_back({*i,expr});
                                                continue;
                                        }
                                        auto stmt = statement_factory(id, processed);
                                        if(!stmt){
                                                finallize(parsed);
                                                return std::vector<ParsedVal>();
                                        }
                                        parsed.push_back({*i,stmt});
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
                
                inline Statement* statement_factory(uint32_t dp, std::vector<ParsedVal>& parsed){
                        return stmt_allocators[dp](parsed);
                }
                
                inline Expression* expression_factory(uint32_t dp, std::vector<ParsedVal>& parsed){
                        return expr_allocators[dp](parsed);
                }
                
                inline uint32_t defRule(ParseValType pvt, std::vector<ParseVal> rule){
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
                        return rules.size() -1;
                }

        public:
                typedef std::vector<ParsedVal> ParseInfo;

                uint32_t getRule(std::wstring what){
                        auto iter = rules_name.find(what);
                        if(iter == rules_name.end())
                                return 0;
                        return iter->second;
                }
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
                        auto tok = move?lex.nextTok():lex.lastTok();
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
                                DBG_TRACE("failed dispatch %d %d",dp,(int)dispatch_types[dp]);
                                return new Expression();
                        }
                        if(!is_rule[dp])
                                return expr_parsers[dp](*this,lex);
                        auto parsed = processRule(dp,tok);
                        if(parsed.size()==0)
                                return nullptr;
                        return expression_factory(dp,parsed);
                }
                
                inline void addData(std::wstring data, std::wstring file){
                        lex.addData(data,file);
                }
                
                void Parse(){
                        delete expectStatement(); // lol. just testing.
                }
                
                inline void showError(){
                        std::wcerr << L"error:" << error << std::endl;
                }
                
                uint32_t defStmt(std::wstring name ,std::vector<ParseVal> rule,stmt_alloc alloc){
                        expr_allocators.push_back(nullptr);
                        stmt_allocators.push_back(alloc);
                        dispatch_types.push_back(false);
                        uint32_t tmp;
                        rules_name[name] =  tmp = defRule(lambda::ParseValType::Statement, rule);
                        return tmp;
                         
                }
                
                uint32_t defExpr(std::wstring name ,std::vector<ParseVal> rule,expr_alloc alloc){
                        stmt_allocators.push_back(nullptr);
                        expr_allocators.push_back(alloc);
                        dispatch_types.push_back(true);
                        return rules_name[name] = defRule(lambda::ParseValType::Expression, rule);
                }
                
                uint32_t defExpr(std::wstring name ,ParseVal begin_tok,expr_parser allocator){
                        assert(begin_tok.type == ParseValType::Token&& "defExpr User Function, begin_tok!=Token");
                        is_rule.push_back(false);
                        expr_parsers.push_back(allocator);
                        stmt_parsers.push_back(nullptr);
                        uint tmp = rules_name[name] = dispatch_by_token[lex.TryRecognize(begin_tok.val)] = rules.size(); // dirty hack; :C
                        dispatch_types.push_back(true);
                        stmt_allocators.push_back(nullptr);
                        expr_allocators.push_back(nullptr);
                        rules.push_back({});
                        return tmp;
                }
                
                uint32_t defStmt(std::wstring name ,ParseVal begin_tok,stmt_parser allocator){
                        assert(begin_tok.type == ParseValType::Token&& "defExpr User Function, begin_tok!=Token");
                        is_rule.push_back(false);
                        stmt_parsers.push_back(allocator);
                        expr_parsers.push_back(nullptr);
                        stmt_allocators.push_back(nullptr);
                        expr_allocators.push_back(nullptr);
                        dispatch_types.push_back(false);
                        uint tmp = rules_name[name] = dispatch_by_token[lex.TryRecognize(begin_tok.val)] = rules.size();
                        rules.push_back({});
                        return tmp;
                }
                void _show_rule(uint32_t id, uint32_t tok){                        
                                std::wcerr << L"is_expr:" << (dispatch_types[id]?L"true\t":L"false\t") << tok << L'\t';
                                if(!is_rule[id]){
                                        std::wcerr << lex.tokById(tok) << L"\t user-spec. parser function\n";
                                        return;
                                }
                                for(auto&x:rules[id]){
                                        if(x.type == ParseValType::Token){
                                                std::wcerr << x.val << L"(" << x.token_id << L")\t";
                                        }else if(x.type == ParseValType::Expression){
                                                std::wcerr << L"$expression\t";
                                        }else if(x.type == ParseValType::Statement){
                                                std::wcerr << L"#statement\t";
                                        }else if(x.type == ParseValType::Rule){
                                                std::wcerr << L"@rule<\t";
                                                if(x.rule_id == id){
                                                        std::wcerr << L"self-recursive\t";
                                                }
                                                else{
                                                        _show_rule(x.rule_id, x.token_id);
                                                }
                                                std::wcerr << L">\t";
                                        }else{                                                
                                                std::wcerr << L"%identifer\t";
                                        }
                                } 
                }
                void showRules(){
                        for(auto&x:dispatch_by_token){
                                _show_rule(x.second, x.first);
                                std::wcerr << std::endl;
                        }
                }
        };
        // i like c++ template magic, so this can simplify way of def syntax rules.
        //this 3 structs is used only in templates matching(like functional pattern matching)
        // wstring      => Token
        // id           => Ident
        // expr         => Expr
        // stmt         => Stmt
        // typeof{Token,Ident,Expr,Stmt} = ParseVal
        // compiler should inline all this functions, no overhead :)
        namespace Syntax{
                struct _SYNTAX_IDENTIFER_{};
                struct _SYNTAX_EXPRESSION{};
                struct _SYNTAX_STATEMENT{};
                _SYNTAX_IDENTIFER_              id;
                _SYNTAX_EXPRESSION              expr;
                _SYNTAX_STATEMENT               stmt;

                inline ParseVal match(std::wstring s){
                        return ParseVal::Token(s);
                }
                inline ParseVal match(uint32_t id){
                        return ParseVal::Rule(id);
                }
                inline ParseVal match(_SYNTAX_IDENTIFER_){
                        return ParseVal::Ident();
                }
                inline ParseVal match(_SYNTAX_EXPRESSION){
                        return ParseVal::Expr();
                }
                inline ParseVal match(_SYNTAX_STATEMENT){
                        return ParseVal::Stmt();
                }
        }
        // std::wstring tok - safes from some errors. syntax rule _must_ starts with uniq token.
        template<typename ... Rest>  inline std::vector<ParseVal> defSyntax(std::wstring tok ,Rest ... rest){
                return std::vector<ParseVal> {Syntax::match(tok),Syntax::match(rest)...};
        }
}
