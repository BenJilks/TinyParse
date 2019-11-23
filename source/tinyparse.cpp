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
    GeneratorCpp gen("test_project/test.hpp");
    Parser *parser = new Parser(lex, gen);
    gen.generate("test");

    lex.report_errors();
    delete parser;
    return 0;
}
