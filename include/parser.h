#ifndef PARSER_H
#define PARSER_H

#include "tinylex.h"
#include "rule.h"

#define TOKEN_LEN       80
#define TOKEN_BUFFER    80

typedef struct _Parser
{
    // Project data
    char project_name[80];

    // Parser data
    Rule *rules;
    int rule_buffer_size;
    int rule_count;

    // Lexer data
    char *tokens;
    int token_buffer;
    int token_count;

    // Commands
    Command *commands;
    int command_count;

    // Compiled table data
    char *table;
    int table_width;
    int table_size;
    int entry_index;
} Parser;

Parser parser_create();
Rule *parser_find_rule(Parser *parser, const char *name);
int parser_find_rule_index(Parser *parser, const char *name);
int parser_find_token_id(Parser *parser, const char *alias);
void parser_parse(Parser *parser, LexerStream *lex);

int parser_compile_and_link(Parser *parser, LexerStream *lex, 
    const char *entry_point);

void parser_free(Parser *parser);

#endif // PARSER_H
