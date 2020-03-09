#include <stdio.h>
#define TINYLEX_IMPLEMENT
#define TINYPARSE_IMPLEMENT
#define TESTPROJECT_IMPLEMENT
#include "lexer.h"
#include "tinyparse.h"

int main()
{
    LexerStream lex;
    Document doc;

    lex = lexer_stream_open("test.txt");
    testproject_init(&lex);
    doc = tinyparse_parse(&lex);

    FunctionNode *func;
    func = (FunctionNode*)doc.root;
//    printf("%s\n", type_names[func->block->type]);

    tinyparse_free_document(&doc);
    lexer_stream_close(&lex);
}
