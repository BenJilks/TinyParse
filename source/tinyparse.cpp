#include <iostream>
#define TINYLEX_IMPLEMENT
#include "tinylex.hpp"
#include "parser.hpp"
#include "rule.hpp"
using namespace TinyParse;

int main()
{
    Lexer lex("test.tinyparse");
    Parser parser(lex);
    //ParseRule rule(lex);
    lex.report_errors();

    return 0;
}
