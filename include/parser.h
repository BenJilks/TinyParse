#ifndef PARSER_H
#define PARSER_H

#include "tinylex.h"
#include "rule.h"

typedef struct _Parser
{
    char project_name[80];

    Rule *rules;
    int rule_buffer_size;
    int rule_count;
} Parser;

Parser parser_create();
Rule *parser_find_rule(Parser *parser, const char *name);
void parser_parse(Parser *parser, LexerStream *lex);
void parser_free(Parser *parser);

#endif // PARSER_H
