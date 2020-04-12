#include <stdio.h>

//#define DEBUG_TINYPARSE
#define TINYLEX_IMPLEMENT
#define TEST_IMPLEMENT
#include "lexer.h"
#include "parser.h"

LexerStream lex;

int indent = 0;
#define LOG(...) \
{ \
	int i; \
	for (i = 0; i < indent; i++) \
		printf("\t"); \
	printf("=> "); \
	printf(__VA_ARGS__); \
	printf("\n"); \
}
#define START_SCOPE	indent += 1;
#define END_SCOPE	indent -= 1;

#include "transverse.c"

int main()
{
	Document doc;
	EntryNode *entry;

	lex = lexer_stream_open("test.txt");
	test_init(&lex);

	doc = test_parse(&lex);
	entry = (EntryNode*)doc.root;
	tr_entry(entry);

	lexer_stream_close(&lex);
	return 0;
}

