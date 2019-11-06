#pragma once
#include "tinylex.hpp"
#include "rule.hpp"

namespace TinyParse
{

    class Parser : public TinyError
    {
    public:
        Parser(Lexer &lex);
        
    private:
        void parse_set(Lexer &lex);
        void parse_rule(Lexer &lex);


    };

}
