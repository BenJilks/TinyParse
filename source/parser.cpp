#include "parser.hpp"
using namespace TinyParse;

Parser::Parser(Lexer &lex)
{
    while (!lex.is_eof())
    {
        switch (lex.get_look().type)
        {
            case Token::TokenType::Set: parse_set(lex);
        }
    }
}

void Parser::parse_set(Lexer &lex)
{
    lex.match(Token::TokenType::Set, "set");
    Token prop = lex.match(Token::TokenType::Keyword, "Name");
    string prop_name = string(lex.read_string(prop.data, prop.length));

    Token value = lex.match(Token::TokenType::Property, "Property");
    string value_data = string(lex.read_string(value.data, value.length));

    std::cout << prop_name << " = " << value_data << std::endl;
}
