#include <stdio.h>
#define TINYLEX_IMPLEMENT
#define TINYPARSE_IMPLEMENT
#define TESTPROJECT_IMPLEMENT
#include "lexer.h"
#include "tinyparse.h"

int main()
{
    LexerStream lex;

    lex = lexer_stream_open("test.txt");
    testproject_init(&lex);
    tinyparse_parse(&lex);

    lexer_stream_close(&lex);
}
