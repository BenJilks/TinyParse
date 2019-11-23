#pragma once
#include "tinylex.hpp"
#include <vector>
#include <string>
#include <tuple>
using std::vector;
using std::string;
using std::pair;

namespace TinyParse
{

    class Rule : public TinyLib::TinyError
    {
    public:
        Rule(Lexer &lex, string name);
        inline const auto &get_name() const { return name; }
        inline const auto &get_labels() const { return labels; }
	inline const auto get_root() const { return root; }

        enum class NodeType
        {
            RuleList,
            Keyword,
            LexNode
        };

        struct Node
        {
            vector<Node*> children;
            string value, label;
            NodeType type;
            bool has_label;
        };

    private:
        vector<pair<string, string>> labels;
        string name;
        Node *root;

        Node *parse_node(Lexer &lex, Node *parent);
        Node *parse_keyword(Lexer &lex, Node *parent);
        Node *parse_lex_node(Lexer &lex, Node *parent);

    };

}
