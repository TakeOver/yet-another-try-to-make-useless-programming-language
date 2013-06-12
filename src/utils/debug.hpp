#pragma once
#include <cstdio>
namespace nls{
        #ifdef DEBUG
                #ifdef USE_PRETTY_FUNCTION
                        #define ___FUNC___ __PRETTY_FUNCTION__
                #else
                        #define ___FUNC___ __FUNCTION__
                #endif
                #define DBG_TRACE(format,...) fprintf(stdout,"file:%s line:%d func:%s msg:" format "\n", __FILE__,__LINE__,___FUNC___, ##__VA_ARGS__)
        #else
                #define DBG_TRACE(a,...) ((void)0)
        #endif
}
