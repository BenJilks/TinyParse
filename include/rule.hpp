#pragma once
#include "tinylex.hpp"
#include <vector>
using std::vector;

namespace TinyParse
{

    class ParseRule
    {
    public:
        ParseRule(Lexer &lex);

        enum class OperationType
        {
            None
        };

        struct Node
        {
            vector<Node*> children;
            string value;
            OperationType op;
        };

    private:
        Node *parse_node(Lexer &lex);

    };

}
