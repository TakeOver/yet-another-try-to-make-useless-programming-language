#pragma once

#include "../lexer/lexer.hpp"
#include "../ast/statement.hpp"
#include "../ast/expression.hpp"
#include "../utils/debug.hpp"
#include "parseval.hpp"
#include <map>
#include <functional>
#include <exception>

namespace lambda{

        class Parser{
                
                typedef std::function < Statement*  ( std::vector<ParsedVal>& ) > stmt_alloc;
                typedef std::function < Expression* ( std::vector<ParsedVal>& ) > expr_alloc;

                typedef std::function < Statement*  ( Parser&, Lexer& ) >       stmt_parser;
                typedef std::function < Expression* ( Parser&, Lexer& ) >       expr_parser;

                std::unordered_map< uint32_t, uint32_t >                        dispatch_by_token;
                std::vector< bool >                                             dispatch_types; // true - expression. false - stmt;
                std::vector< stmt_alloc >                                       stmt_allocators;
                std::vector< expr_alloc >                                       expr_allocators; 
                std::vector< std::vector< ParseVal > >                          rules; 
                std::vector< bool >                                             is_rule; // is rule or  user function .
                std::vector< stmt_parser >                                      stmt_parsers;
                std::vector< expr_parser >                                      expr_parsers;
                std::unordered_map< std::wstring, uint32_t >                    rules_name;
                std::vector<std::wstring>                                       rules_name_by_id;

                Lexer lex;
                
                bool            failed = false;
                std::wstring    error;
                
                inline void HandleError(std::wstring msg, tok_info_t ti){

                        DBG_TRACE();

                        failed = true;

                        lex.HandleError(msg,ti);
                        error = lex.error;
                }
                
                inline void ForceLexerError(){
                        failed = lex.failed;
                        error = lex.error;
                }
                
                inline uint32_t dispatch(token_t & tok){
                        auto iter = dispatch_by_token.find(tok.id);

                        if( iter == dispatch_by_token.end())
                                return 0;

                        return iter->second;
                }
                
                static inline void finallize(std::vector<ParsedVal> vec){                        
                        for(auto&x:vec){                        
                                if(x.pv.type == ParseValType::Expression){                        
                                        delete x.expr;                        
                                }else if(x.pv.type == ParseValType::Statement){                        
                                        delete x.stmt;                        
                                }                        
                        }
                }
                
                std::vector<ParsedVal> processRule(uint32_t dp, token_t & tok){       

                        std::vector< ParsedVal > parsed;

                        auto& rule = rules[ dp ];

                        parsed.push_back( { rule.front() } );

                        DBG_TRACE( "rule:%d", dp );

                        for(std::vector< ParseVal >::iterator i = rule.begin() + 1, e = rule.end(); i!=e; ++i ){

                                DBG_TRACE( "rule:%d token_id:%d type:%s", dp, i->token_id, ParseVal::type2str( i->any, i->type ).c_str() );

                                if( i->type == ParseValType::Token){
                                        auto token = lex.nextTok();
                                        
                                        if( token.id != i->token_id ){
                                                HandleError( L"Expected: " + i->val + L", found:" + token.val, token.tokinfo );
                                                finallize( parsed );
                                                return std::vector< ParsedVal >();
                                        }
                                        
                                        parsed.push_back( ParsedVal( *i ) );
                                        
                                        continue;
                                }

                                if( i->type == ParseValType::Identifer){
                                        
                                        bool const any = i->any;
                                        
                                        std::vector<token_t> ids;
                                        
                                        while(true){
                                                auto id = lex.nextTok();
                                        
                                                if( id.tok!=Token::IDENTIFER || id.id!=static_cast< uint32_t >( id.tok ) ){
                                                        lex.decCachePos();
                                                        break;
                                                }
                                        
                                                ids.push_back(id);
                                        
                                        }
                                        
                                        if( !any && !ids.size() ){
                                                HandleError( L"Identifer expected, found:" + lex.lookNextTok().val, 
                                                        lex.lookNextTok().tokinfo );
                                                finallize( parsed );
                                                return std::vector< ParsedVal >();
                                        }
                                        
                                        parsed.push_back( { *i, ids } );
                                        
                                        continue;
                                }

                                if( i->type == ParseValType::Expression){
                                        
                                        std::vector< Expression* > st;
                                        
                                        const bool any = i->any;
                                        
                                        while(true){
                                                auto _tok = lex.lookNextTok();
                                                auto id = dispatch( _tok );
                                                if( id && !dispatch_types[ id ] )
                                                        break;
                                                
                                                auto expr = expectExpression();
                                                if( !expr ){
                                                        finallize( parsed );
                                                        return std::vector< ParsedVal >();
                                                }
                                                st.push_back( expr );
                                                
                                                if( !any ){
                                                        break;
                                                }

                                        }
                                        
                                        if ( !any && !st.size() ){
                                                HandleError( L"Expression expected, found:" + lex.lastTok().val, lex.lastTok().tokinfo );
                                                finallize( parsed );
                                                return std::vector< ParsedVal >();
                                        }
                                        
                                        Expression ** data = new Expression*[ st.size() + 1 ]();
                                        
                                        for (uint64_t i = 0, e = st.size() ; i < e ; ++i ) {
                                                data[ i ] = st[ i ];
                                        }
                                        
                                        data[ st.size() ] = nullptr;
                                        
                                        parsed.push_back( { *i, data, static_cast< uint32_t >( st.size() ) } );
                                        
                                        continue;
                                }

                                if( i->type == ParseValType::Rule ){
                                        
                                        auto id = i->rule_id;
                                        
                                        const bool any = i->any;
                                        
                                        if( !id ){
                                                HandleError( L"Rule expected(id == 0)", lex.lastTok().tokinfo );
                                                finallize( parsed );
                                                return std::vector< ParsedVal >();
                                        }
                                        
                                        const ParseVal rule_begin = rules[ id ].front();

                                        // nested struct. is used only as ptr-holder. type - constant. so, i'll have no collisions with statements and epxression and this code would work well
                                        struct __tmp {
                                                union{
                                                        Expression      * ptre  = nullptr;
                                                        Statement       * ptrs  = nullptr;
                                                        void            * ptr   = nullptr;
                                                };
                                                __tmp(void*ptr):ptr(ptr){}
                                        };

                                        auto const type = dispatch_types[ id ];

                                        std::vector< __tmp > vec;

                                        while(true){
                                                token_t _tok = lex.nextTok();

                                                if( rule_begin.token_id != _tok.id ){
                                                        lex.decCachePos();
                                                        break;
                                                }

                                                auto processed = processRule( id, _tok );
                                                if(processed.size() == 0){
                                                        finallize( parsed );                                        
                                                        for( auto&x : vec )
                                                                if( type ){
                                                                        delete x.ptre; 
                                                                } else {
                                                                        delete x.ptrs;
                                                                }                                        
                                                        return std::vector< ParsedVal >();
                                                }

                                                if(type){
                                                        auto expr = expression_factory( id, processed );                                        
                                                        if( !expr ){
                                                                finallize( parsed );
                                                                for( auto&x : vec )
                                                                        if( type ){
                                                                                delete x.ptre; 
                                                                        } else {
                                                                                delete x.ptrs;
                                                                        }
                                                                return std::vector< ParsedVal >();
                                                        }                                        
                                                        vec.push_back( expr );
                                                        
                                                        if( any ){
                                                                continue;
                                                        }else{   
                                                                break;
                                                        }

                                                }

                                                auto stmt = statement_factory( id, processed );

                                                if( !stmt ){
                                                        finallize( parsed );                                                        
                                                        for( auto& x : vec ){
                                                                if( type ){
                                                                        delete x.ptre; 
                                                                } else {
                                                                        delete x.ptrs;
                                                                }
                                                        }                                                        
                                                        return std::vector< ParsedVal >();
                                                }

                                                vec.push_back( stmt );

                                                if( any ){
                                                        continue;
                                                }else{   
                                                        break;
                                                }

                                        }

                                        if( !any && !vec.size() ){
                                                HandleError( L"Rule expected[" + rule_begin.val + L"] found:" + lex.lookNextTok().val, 
                                                        lex.lookNextTok().tokinfo );
                                                finallize( parsed );

                                                for( auto& x : vec ){
                                                        if( type ){
                                                                delete x.ptre; 
                                                        } else {
                                                                delete x.ptrs;
                                                        }
                                                }                                                
                                                return std::vector< ParsedVal >();

                                        }

                                        if(type){
                                                Expression ** data = new Expression* [ vec.size() + 1 ];

                                                for( uint64_t i = 0; i < vec.size(); ++i )
                                                        data[ i ] = vec[ i ].ptre;

                                                data[ vec.size() ] = nullptr;
                                                parsed.push_back( { *i, data, static_cast< uint32_t >( vec.size() ) } );

                                        }else{
                                                Statement ** data = new Statement* [ vec.size() + 1 ];

                                                for( uint64_t i = 0; i < vec.size(); ++i )
                                                        data[ i ] = vec[ i ].ptrs;

                                                data[ vec.size() ] = nullptr;
                                                parsed.push_back( { *i, data, static_cast< uint32_t >( vec.size() ) } );
                                        }
                                        continue;
                                }

                                if( i->type == ParseValType::String){
                                        bool const any = i->any;
                                        
                                        std::vector<token_t> ids;
                                        
                                        while(true){
                                                auto id = lex.nextTok();
                                                if( id.tok != Token::STRING ){
                                                        lex.decCachePos();
                                                        break;
                                                }
                                                ids.push_back( id );
                                        
                                        }
                                        
                                        if( !any && !ids.size() ){
                                                HandleError( L"String expected, found:" + lex.lookNextTok().val, 
                                                        lex.lookNextTok().tokinfo );
                                                finallize( parsed );
                                                return std::vector< ParsedVal >();
                                        }

                                        parsed.push_back( { *i, ids } );
                                        
                                        continue;
                                }

                                if( i->type == ParseValType::Operator ){                                        
                                        bool const any = i->any;
                                        
                                        std::vector<token_t> ids;
                                        
                                        while(true){
                                                auto id = lex.nextTok();
                                                if( id.tok!=Token::OPERATOR){
                                                        lex.decCachePos();
                                                        break;
                                                }
                                                ids.push_back(id);
                                        
                                        }

                                        if( !any && !ids.size() ){
                                                HandleError( L"Operator expected, found:" + lex.lookNextTok().val, lex.lookNextTok().tokinfo);
                                                finallize( parsed );
                                                return std::vector< ParsedVal >();
                                        }
                                        
                                        parsed.push_back({*i, ids});
                                        
                                        continue;
                                }

                                if( i->type == ParseValType::Number){                                        
                                        bool const any = i->any;
                                        
                                        std::vector<token_t> ids;
                                        
                                        while(true){
                                                auto id = lex.nextTok();
                                                if( id.tok!=Token::NUM){
                                                        lex.decCachePos();
                                                        break;
                                                }
                                                ids.push_back(id);
                                        
                                        }
                                        
                                        if( !any && !ids.size() ){
                                                HandleError( L"Number expected, found:" + lex.lookNextTok().val, lex.lookNextTok().tokinfo);
                                                finallize( parsed );
                                                return std::vector< ParsedVal >();
                                        }
                                        
                                        parsed.push_back({*i, ids});
                                        
                                        continue;
                                }

                                if( i->type == ParseValType::Term){                                                              
                                        bool const any = i->any;
                                        
                                        std::vector<token_t> ids;
                                        
                                        while(true){
                                                auto id = lex.nextTok();
                                                if( id.tok!=Token::NUM && id.tok!=Token::STRING && static_cast<Token>(id.id)!=Token::OPERATOR  && (id.tok!=Token::IDENTIFER || static_cast<uint32_t>(id.tok) != id.id) ){
                                                        lex.decCachePos();
                                                        break;
                                                }
                                                ids.push_back(id);
                                        
                                        }
                                        
                                        if( !any && !ids.size() ){
                                                HandleError( L"Term expected, found:" + lex.lookNextTok().val, lex.lookNextTok().tokinfo);
                                                finallize( parsed );
                                                return std::vector< ParsedVal >();
                                        }
                                        
                                        parsed.push_back({*i, ids});
                                        
                                        continue;
                                }

                                if( i->type == ParseValType::Statement){                                        
                                        DBG_TRACE("parsing statements*");
                                        
                                        std::vector<Statement*> st;
                                        
                                        const bool any = i->any;
                                        
                                        while(true){
                                                auto _tok = lex.lookNextTok();
                                                auto id = dispatch(_tok);
                                        
                                                if( !id){
                                                        break;
                                                }
                                                
                                                auto stmt = expectStatement();
                                                if( !stmt){
                                                        finallize( parsed );
                                                        return std::vector< ParsedVal >();
                                                }
                                                
                                                st.push_back(stmt);
                                                
                                                if( !any){
                                                        break;
                                                }

                                        }
                                        
                                        if( !any && !st.size() ){
                                                HandleError( L"Statement expected, found:"+ lex.lastTok().val, lex.lastTok().tokinfo);
                                                finallize( parsed );
                                                return std::vector< ParsedVal >();
                                        }
                                        
                                        Statement ** data = new Statement*[st.size()+1]();
                                        
                                        for(uint i=0, e = st.size();i<e;++i){
                                                data[i] = st[i];
                                        }
                                        
                                        data[st.size()] = nullptr;
                                        parsed.push_back({*i, data, static_cast<uint32_t>(st.size())});
                                        
                                        continue;
                                }

                                assert(false);
                                return std::vector< ParsedVal >();   

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
                        
                        if(rule.size()==0){// FIXME
                                throw  std::string("Rule size  == 0");
                        }

                        if(rule.front().type!=ParseValType::Token){
                                throw std::string("First element of rule != Token");
                        }
                        
                        for(auto&x:rule){                        
                                if(x.type == ParseValType::Token){
                                        x.token_id = lex.TryRecognize(x.val);
                                }else if(x.type == ParseValType::SelfRecursive){
                                        x.rule_id = this->rulesCount();
                                        x.type = ParseValType::Rule;
                                }
                        
                        }
                        
                        auto id = rule.front().token_id;
                        dispatch_by_token[id] = rules.size();
                        rules.push_back(rule);
                        
                        return rules.size() -1;
                }

        public:
                typedef std::vector < ParsedVal > ParseInfo;

                uint32_t getRule( std::wstring what ){                        
                        auto iter = rules_name.find( what );
                        
                        if ( iter == rules_name.end() ){
                                return 0;
                        }
                        
                        return iter->second;
                }

                Parser(){                        
                        rules.push_back( {} );

                        rules_name_by_id.push_back(L"");
                        
                        
                        expr_allocators.push_back( nullptr );
                        stmt_allocators.push_back( nullptr );

                        stmt_parsers.push_back( nullptr );
                        expr_parsers.push_back( nullptr );

                        dispatch_types.push_back( false );
                        
                        is_rule.push_back( true );
                }
                
                Statement* expectStatement(bool move = true){                        
                        DBG_TRACE();
                        
                        ForceLexerError();
                        if ( failed )
                                return nullptr;
                        
                        auto tok = (move) ? (lex.nextTok()) : (lex.lastTok());
                        
                        auto dp = dispatch( tok );
                        
                        if (!dp || dispatch_types[ dp ])
                                return new Statement( expectExpression( false ) ); // hack. 
                        
                        if ( !is_rule[ dp ] )
                                return stmt_parsers[dp]( *this, lex );
                        
                        auto parsed = processRule( dp, tok );
                        if ( parsed.size() == 0 )
                                return nullptr;
                        
                        return statement_factory( dp, parsed );
                }
                
                Expression * expectExpression(bool move = true){                        
                        DBG_TRACE();
                        
                        ForceLexerError();
                        if(failed)
                                return nullptr;
                        
                        auto tok = move?lex.nextTok():lex.lastTok();
                        auto dp = dispatch(tok);
                        
                        if( !dp || dispatch_types[dp]==false){
                                //TODO EXPR PARSING.
                                DBG_TRACE("failed dispatch %d %d",dp,(int)dispatch_types[dp]);
                                return new Expression();
                        }
                        
                        if( !is_rule[dp]){
                                return expr_parsers[dp](*this, lex);
                        }
                        
                        auto parsed = processRule(dp,tok);
                        if(parsed.size()==0){
                                return nullptr;
                        }
                        
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
                        
                        rules_name_by_id.push_back(name);
                        
                        dispatch_types.push_back(false);
                        
                        uint32_t tmp;
                        rules_name[name] =  tmp = defRule(lambda::ParseValType::Statement, rule);
                        
                        return tmp;
                         
                }
                
                uint32_t defExpr(std::wstring name ,std::vector<ParseVal> rule,expr_alloc alloc){                        
                        stmt_allocators.push_back(nullptr);
                        expr_allocators.push_back(alloc);
                        
                        rules_name_by_id.push_back(name);
                        
                        dispatch_types.push_back(true);
                        
                        return rules_name[name] = defRule(lambda::ParseValType::Expression, rule);
                }
                
                uint32_t defExpr(std::wstring name ,ParseVal begin_tok,expr_parser allocator){                        
                        assert(begin_tok.type == ParseValType::Token&& "defExpr User Function, begin_tok!=Token");
                        
                        rules_name_by_id.push_back(name);
                        
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
                        
                        rules_name_by_id.push_back(name);

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
                        std::wcerr << (dispatch_types[id]?L"[expr] [id:":L"[stmt] [id:") << id << L"] ";
                
                        if( !is_rule[id]){
                                std::wcerr << lex.tokById(tok) <<L"(" << tok << L")" << L" <function>\n";                                        return;
                        }
                        
                        for(auto&x:rules[id]){                        
                                if(x.type == ParseValType::Token){
                                        std::wcerr << x.val/* << L"(" << x.token_id << L") "*/ << L" ";
                                }else if(x.type == ParseValType::Expression){
                                        std::wcerr << (x.any?L"any!":L"") <<  L"$expr ";
                                }else if(x.type == ParseValType::Statement){
                                        std::wcerr << (x.any?L"any!":L"") << L"#stmt ";
                                }else if(x.type == ParseValType::Rule){                        
                                        std::wcerr << (x.any?L"any!":L"") << L"@rule< ";
                                        if(x.rule_id == id){
                                                std::wcerr << L"self! ";
                                        }
                                        else{
                                                _show_rule(x.rule_id, rules[x.rule_id].front().token_id);
                                        }
                
                                        std::wcerr << L"> ";

                                }else if(x.type == ParseValType::Term){
                                        std::wcerr << (x.any?L"any!":L"") << L"#term ";
                                }else{                                                
                                        std::wcerr << (x.any?L"any!":L"") << L"%id ";
                                }
                        } 
                }
                void ShowRuleBNF(uint32_t id, uint32_t tok){
                        std::wcerr << (dispatch_types[id]?L"[expr] <":L"[stmt] <") << rules_name_by_id[id] << L"> ::= ";                        
                        if( !is_rule[id] ){
                                std::wcerr << "\'" << lex.tokById(tok) <<L"\' " << L" <user-function>\n";
                                return;
                        }
                        for( auto& x : rules[ id ] ){
                                using i = ParseValType;
                                auto& j = std::wcerr;
                                switch(x.type){
                                        case i::Token: j << L'\"' << lex.tokById(x.token_id) << L'\"'; break; 
                                        case i::Identifer: j << L"<id>"; break;
                                        case i::Statement: j << L"<stmt>"; break;
                                        case i::Expression: j << L"<expr>"; break;
                                        case i::String: j << L"<str>"; break;
                                        case i::Number: j << L"<num>"; break;
                                        case i::Rule: j << L"<" << rules_name_by_id[x.rule_id] << L">"; break;
                                        case i::Term: j << L"<term>"; break;
                                        default: j << L"<$err>"; break;
                                }
                                if(x.any){
                                        j << L"*";
                                }
                                j << L" ";
                        }

                }
                const uint32_t rulesCount()const{
                        return rules.size();
                }

                void showRules(){                        
                        for(auto&x:dispatch_by_token){        
                                std::wcerr << L"[id:" << x.second << L"] ";                 
                                ShowRuleBNF(x.second, x.first);                        
                                std::wcerr << std::endl;                        
                        }
                }
        };
}
