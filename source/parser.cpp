#include "parser.hpp"
using namespace TinyParse;

Parser::Parser(Lexer &lex, Generator &gen)
{
    while (!lex.is_eof())
    {
        switch (lex.get_look().type)
        {
            case Token::TokenType::Define: parse_define(lex, gen); break;
            case Token::TokenType::Set: parse_set(lex, gen); break;
            case Token::TokenType::Keyword: parse_rule(lex, gen); break;
            default: 
                lex.error("Unexpected token", lex.get_look());
                lex.next();
        }
    }
}

void Parser::parse_define(Lexer &lex, Generator &gen)
{
    lex.match(Token::TokenType::Define, "define");
    string name = lex.read_data(lex.next());
    lex.match(Token::TokenType::As, "as");
    string value = lex.read_data(lex.next());

    gen.add_define(name, value);
}

void Parser::parse_set(Lexer &lex, Generator &gen)
{
    lex.match(Token::TokenType::Set, "set");
    Token prop = lex.match(Token::TokenType::Keyword, "Name");
    string prop_name = lex.read_data(prop);

    Token value = lex.match(Token::TokenType::Property, "Property");
    string value_data = lex.read_data(value);

    std::cout << prop_name << " = " << value_data << std::endl;
}

void Parser::parse_rule(Lexer &lex, Generator &gen)
{
    Token name = lex.match(Token::TokenType::Keyword, "Rule Name");
    string rule_name = lex.read_data(name);
    Rule rule(lex, rule_name);
    gen.add_rule(rule);

    std::cout << "Read rule '" << rule_name << "'" << std::endl;
}
