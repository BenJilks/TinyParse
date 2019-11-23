#include "rule.hpp"
using namespace TinyParse;

Rule::Rule(Lexer &lex, string name) :
    name(name)
{
    parse_node(lex, nullptr);
}

Rule::Node *Rule::parse_node(Lexer &lex, Rule::Node *parent)
{
    Node *node = new Node;
    node->type = NodeType::RuleList;

    lex.match(Token::TokenType::Open, "(");
    while (lex.get_look().type != Token::TokenType::Close)
    {
        switch (lex.get_look().type)
        {
            case Token::TokenType::Keyword:
            case Token::TokenType::Symbol: parse_keyword(lex, node); break;
            case Token::TokenType::OpenNode: parse_lex_node(lex, node); break;
            default:
                lex.error("Unexpected token", lex.get_look());
                lex.next();
        }
    }
    lex.match(Token::TokenType::Close, ")");

    if (parent != nullptr)
        parent->children.push_back(node);
    return node;
}

Rule::Node *Rule::parse_keyword(Lexer &lex, Rule::Node *parent)
{
    // Read the single keyword or symbol token
    Node *node = new Node;
    node->type = NodeType::Keyword;
    node->value = lex.read_data(lex.next());
    node->has_label = false;
    parent->children.push_back(node);
    return node;
}

Rule::Node *Rule::parse_lex_node(Lexer &lex, Rule::Node *parent)
{
    Node *node = new Node;
    node->type = NodeType::LexNode;
    node->has_label = false;

    lex.match(Token::TokenType::OpenNode, "<");
    node->value = lex.read_data(lex.next());
    if (lex.get_look().type == Token::TokenType::Named)
    {
        lex.match(Token::TokenType::Named, ":");
        node->label = lex.read_data(lex.next());
        node->has_label = true;
        labels.push_back(std::make_pair(node->label, node->value));
    }
    lex.match(Token::TokenType::CloseNode, ">");

    parent->children.push_back(node);
    return node;
}
