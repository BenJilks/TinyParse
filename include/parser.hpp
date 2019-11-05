#pragma once
#include "tinylex.hpp"

namespace TinyParse
{

    class Parser : public TinyError
    {
    public:
        Parser(Lexer &lex);
        
    private:
        void parse_set(Lexer &lex);

        string test;

    };

}
