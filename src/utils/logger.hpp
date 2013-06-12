#pragma once
#include "../utils/json.hpp"
namespace lambda{
        enum class LogLvl{
                WARN = 0,
                FATAL,
                IGNORE
        };
        enum class MsgType{
                None = 0,
                Exception,
                Runtime,
                Compile,
                Debug
        };
        class Logger: public boost::noncopyable {
                bool enable, debug_enable;
                LogLvl lvl;
                std::wstring dir;
                std::wstring comp, runt, excep, result,debug;
                static Logger logger;
                JsonWriter comp_jw, runt_jw, excep_jw, result_jw, debug_jw;
                uint resno = 0, compno = 0, runtno = 0, excepno = 0, debugno =0;
                ~Logger(){
                        char* s = new char[100];
                        std::wcstombs(s, comp.c_str(), 99);
                        comp_jw.write2File(s);
                        std::wcstombs(s, result.c_str(), 99);
                        result_jw.write2File(s);
                        std::wcstombs(s, runt.c_str(), 99);
                        runt_jw.write2File(s);
                        std::wcstombs(s, excep.c_str(), 99);
                        excep_jw.write2File(s);
                        if(debug_enable){
                                std::wcstombs(s, debug.c_str(), 99);
                                debug_jw.write2File(s);
                        }
                        delete [] s;
                }
                Logger(){
                        JsonReader jr ("./config/log.json");
                        enable = jr.get(L"enable", true);
                        debug_enable = jr.get(L"debug_enable", false);
                        lvl = static_cast<LogLvl>(jr.get(L"log_level",1));
                        dir = jr.get(L"dir",std::wstring(L"./"));
                        comp = jr.get(L"file_compile",std::wstring(L"compile_err.json"));
                        runt = jr.get(L"file_runtime",std::wstring(L"runtime_err.json"));
                        excep = jr.get(L"file_exceptions",std::wstring(L"exceptions.json"));
                        result = jr.get(L"file_results",std::wstring(L"results.json"));
                        if(debug_enable)
                                debug = jr.get(L"file_debug",std::wstring(L"debug.json"));
                }
        public:

                static void log(MsgType mt, LogLvl ll, std::wstring msg){
                        if(ll == LogLvl::IGNORE)
                                return;
                        std::wstring sub;
                        if(ll == LogLvl::FATAL)
                                sub = L"Fatal:";
                        else 
                                sub = L"Warning:";
                        switch (mt){
                                case MsgType::None:
                                        logger.result_jw.add(std::to_wstring(logger.resno++), sub+msg);
                                        break;
                                case MsgType::Runtime:
                                        logger.runt_jw.add(std::to_wstring(logger.runtno++), sub+msg);
                                        break;
                                case MsgType::Compile:
                                        logger.comp_jw.add(std::to_wstring(logger.compno++), sub+msg);
                                        break;
                                case MsgType::Exception:
                                        logger.excep_jw.add(std::to_wstring(logger.excepno++), sub+msg);
                                        break;
                                case MsgType::Debug:
                                        if(logger.debug_enable)
                                                logger.debug_jw.add(std::to_wstring(logger.debugno++), sub+msg);
                                        break;
                                default: break;
                        }
                        if(ll == LogLvl::FATAL){
                                throw std::runtime_error("Fatal. Logger::log.");
                        }

                }
        };
}