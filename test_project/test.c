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
    tinyparse_debug_table();
    doc = tinyparse_parse(&lex);

    int count = 1;
    FunctionNode *node = (FunctionNode*)doc.root;
    BlockNode *block = node->block;
    MultiBlockNode *multi_block = block->multi_block;
    while (multi_block->has_next)
    {
        count += 1;
        multi_block = multi_block->next;
    }
    printf("Count: %i\n", count);

    tinyparse_free_document(&doc);
    lexer_stream_close(&lex);
}
