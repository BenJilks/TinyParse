#include "rule.hpp"
using namespace TinyParse;

ParseRule::ParseRule(Lexer &lex)
{
    parse_node(lex);
}

ParseRule::Node *ParseRule::parse_node(Lexer &lex)
{
    lex.match(Token::TokenType::Open, "(");
    while (lex.get_look().type != Token::TokenType::Close)
    {
        printf("%i - %s\n", lex.get_look().data, lex.get_look().type_name);
        lex.next();
    }
    lex.match(Token::TokenType::Close, ")");
}
