#include "parser.hpp"
using namespace TinyParse;

Parser::Parser(Lexer &lex)
{
    while (!lex.is_eof())
    {
        switch (lex.get_look().type)
        {
            case Token::TokenType::Set: parse_set(lex); break;
            case Token::TokenType::Keyword: parse_rule(lex); break;
            default: 
                lex.error("Unexpected token", lex.get_look());
                lex.next();
        }
    }
}

void Parser::parse_set(Lexer &lex)
{
    lex.match(Token::TokenType::Set, "set");
    Token prop = lex.match(Token::TokenType::Keyword, "Name");
    string prop_name = lex.read_data(prop);

    Token value = lex.match(Token::TokenType::Property, "Property");
    string value_data = lex.read_data(value);

    std::cout << prop_name << " = " << value_data << std::endl;
}

void Parser::parse_rule(Lexer &lex)
{
    Token name = lex.match(Token::TokenType::Keyword, "Rule Name");
    Rule rule(lex);

    string rule_name = lex.read_data(name);
    std::cout << "Read rule '" << rule_name << "'" << std::endl;
}
