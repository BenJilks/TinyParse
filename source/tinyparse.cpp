#include <iostream>
#include "generator/generator_cpp.hpp"
#define TINYLEX_IMPLEMENT
#include "tinylex.hpp"
#include "parser.hpp"
#include "rule.hpp"
using namespace TinyParse;

int main()
{
    Lexer lex("test.tinyparse");
    GeneratorCpp gen("test.hpp");
    Parser parser(lex, gen);
    gen.generate("test");

    lex.report_errors();
    return 0;
}
