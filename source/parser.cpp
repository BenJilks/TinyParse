#include "parser.hpp"
using namespace TinyParse;

Parser::Parser(Lexer &lex, Generator &gen)
{
    while (!lex.is_eof())
    {
        switch (lex.get_look().type)
        {
            case Token::TokenType::Set: parse_set(lex, gen); break;
            case Token::TokenType::Keyword: parse_rule(lex, gen); break;
            default: 
                lex.error("Unexpected token", lex.get_look());
                lex.next();
        }
    }
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
    rules.push_back(rule);
    printf("%i\n", rule_name.length());

    Rule *ref = &rules[rules.size() - 1];
    add_dependency(ref);
    gen.add_rule(ref);

    std::cout << "Read rule '" << rule_name << "'" << std::endl;
}
