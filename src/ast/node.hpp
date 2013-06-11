
#pragma once
#include "type.hpp"
#include <string>
namespace lambda{


        class AstNode{
        protected:
                Type type = Type::Unit;
                Registry_t registry = 0;
                bool is_constant = false;
                std::wstring line;
                uint32_t lineno;
        };
}