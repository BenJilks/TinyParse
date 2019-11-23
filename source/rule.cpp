#include "rule.hpp"
using namespace TinyParse;

Rule::Rule(Lexer &lex, string name) :
    name(name)
{
    root = parse_node(lex, nullptr);
}

Rule::Node *Rule::get_entry_node(Node *node)
{
    if (node->type == NodeKeyword || node->type == NodeToken)
        return node;
    
    if (node->children.size() > 0)
        return get_entry_node(node->children[0]);
    
    return nullptr;
}

Rule::Node *Rule::parse_node(Lexer &lex, Rule::Node *parent)
{
    Node *node = new Node;
    node->type = NodeType::NodeList;

    lex.match(Token::Open, "(");
    while (lex.get_look().type != Token::Close)
    {
        switch (lex.get_look().type)
        {
            case Token::Keyword:
            case Token::Define:
            case Token::As:
            case Token::Set:
            case Token::Symbol: parse_keyword(lex, node); break;
            case Token::OpenNode: parse_lex_node(lex, node); break;
            case Token::Optional: parse_mod(lex, node, NodeOptional); break;
            case Token::Any: parse_mod(lex, node, NodeAny); break;
            case Token::OneOrMore: parse_mod(lex, node, NodeOneOrMore); break;
            case Token::Open: parse_node(lex, node); break;
            default:
                lex.error("Unexpected token", lex.get_look());
                lex.next();
        }
    }
    lex.match(Token::Close, ")");

    if (parent != nullptr)
        parent->children.push_back(node);
    return node;
}

Rule::Node *Rule::parse_keyword(Lexer &lex, Rule::Node *parent)
{
    // Read the single keyword or symbol token
    Node *node = new Node;
    node->type = NodeType::NodeKeyword;
    node->value = lex.read_data(lex.next());
    parent->children.push_back(node);
    return node;
}

void Rule::add_label(Node *label)
{
    for (auto &l : labels)
        if (l->label == label->label)
            return;
    
    labels.push_back(label);
}

Rule::Node *Rule::parse_lex_node(Lexer &lex, Rule::Node *parent)
{
    Node *node = new Node;
    node->type = NodeType::NodeToken;
    node->has_label = false;
    node->is_array = false;

    lex.match(Token::OpenNode, "<");
    node->value = lex.read_data(lex.next());
    if (lex.get_look().type == Token::Named)
    {
        lex.match(Token::Named, ":");
        node->label = lex.read_data(lex.next());
        node->has_label = true;

        if (lex.get_look().type == Token::Any)
        {
            lex.next();
            node->is_array = true;
        }
        add_label(node);
    }
    lex.match(Token::CloseNode, ">");

    parent->children.push_back(node);
    return node;
}

Rule::Node *Rule::parse_mod(Lexer &lex, Node *parent, NodeType type)
{
    lex.next();
    Node *option = parent->children.back();
    parent->children.pop_back();

    Node *node = new Node;
    node->children.push_back(option);
    node->type = type;
    parent->children.push_back(node);
    return node;
}
