#pragma once
#include <cstdio>
namespace nls{
        #ifdef DEBUG
                #define DBG_TRACE(format,...) fprintf(stderr,"file:%s line:%d func:%s msg:" format "\n", __FILE__,__LINE__,__PRETTY_FUNCTION__, ##__VA_ARGS__)
        #else
                #define DBG_TRACE(a,...) ((void)0)
        #endif
}