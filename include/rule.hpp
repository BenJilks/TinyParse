#pragma once
#include "tinylex.hpp"
#include <vector>
using std::vector;

namespace TinyParse
{

    class Rule
    {
    public:
        Rule(Lexer &lex);

        enum class NodeType
        {
            RuleList,
            Keyword,
            LexNode
        };

        struct Node
        {
            vector<Node*> children;
            string value;
            NodeType type;
        };

    private:
        Node *parse_node(Lexer &lex, Node *parent);
        Node *parse_keyword(Lexer &lex, Node *parent);
        Node *parse_lex_node(Lexer &lex, Node *parent);

    };

}
