#pragma once
#include "node.hpp"
#include "expression.hpp"
namespace lambda{
        class Statement: protected AstNode{
        public:
                Statement( Expression *){}
                Statement(){}
        };
}