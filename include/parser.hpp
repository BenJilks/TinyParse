#pragma once
#include "generator/generator.hpp"
#include "tinylex.hpp"
#include "rule.hpp"

namespace TinyParse
{

    class Parser : public TinyLib::TinyError
    {
    public:
        Parser(Lexer &lex, Generator &gen);
        
    private:
        vector<Rule> rules;

        void parse_set(Lexer &lex, Generator &gen);
        void parse_rule(Lexer &lex, Generator &gen);


    };

}
