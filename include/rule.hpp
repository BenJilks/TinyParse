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

        enum NodeType
        {
            NodeList,
            NodeKeyword,
            NodeToken,
            NodeOptional,
            NodeAny,
            NodeOneOrMore
        };

        struct Node
        {
            vector<Node*> children;
            string value, label;
            NodeType type;
            bool has_label, is_array;
        };

        static Node *get_entry_node(Node *node);

    private:
        vector<Node*> labels;
        string name;
        Node *root;

        void add_label(Node *label);
        Node *parse_node(Lexer &lex, Node *parent);
        Node *parse_keyword(Lexer &lex, Node *parent);
        Node *parse_lex_node(Lexer &lex, Node *parent);
        Node *parse_mod(Lexer &lex, Node *parent, NodeType type);

    };

}
