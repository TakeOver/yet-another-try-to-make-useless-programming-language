#pragma once
#include <cstdint>
namespace lambda{
        enum class Type{
                Unit = 0,
                Integer,
                Float,
                Char,
                String,
                Array,
                List,
                Tuple
        };        
        typedef uint64_t Registry_t; // SSA.
}