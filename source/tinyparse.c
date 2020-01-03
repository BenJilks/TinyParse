#include <stdio.h>
#include "parser.h"
#include "generator.h"

#define TINYLEX_IMPLEMENT
#define TINYPARSE_IMPLEMENT
#include "tinylex.h"

int main()
{
    LexerStream lex;
    Parser parser;
    FILE *output;

    lex = lexer_stream_open("../test.tinyparse");
    parser = parser_create();
    output = fopen("test.h", "wb");

    tinyparse_init(&lex);
    parser_parse(&parser, &lex);
    parser_compile_and_link(&parser, &lex);
    generate_c(output, &lex, &parser);

    parser_free(&parser);
    lexer_stream_close(&lex);
    fclose(output);
    return 0;
}
