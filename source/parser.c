#include "parser.h"

#define RULE_BUFFER_SIZE 80

Parser parser_create()
{
    Parser parser;
    parser.rules = malloc(RULE_BUFFER_SIZE * sizeof(Rule));
    parser.rule_buffer_size = RULE_BUFFER_SIZE;
    parser.rule_count = 0;

    return parser;
}

Rule *parser_find_rule(
    Parser *parser, 
    const char *name)
{
    int i;

    for (i = 0; i < parser->rule_count; i++)
    {
        if (!strcmp(parser->rules[i].name, name))
            return &parser->rules[i];
    }
    
    return NULL;
}

static void parse_project_name(
    Parser *parser,
    LexerStream *lex)
{
    Token name;
    char *data;

    tinyparse_match(lex, TinyParse_Project, "project");
    name = tinyparse_match(lex, TinyParse_Name, "Name");
    data = lexer_read_buffer(lex, name.data_index, name.len);
    parser->project_name[name.len] = '\0';
    memcpy(parser->project_name, data, name.len);
}

static void parse_rule(
    Parser *parser,
    LexerStream *lex)
{
    Rule rule;
    char test[80];

    // Parse and add rule
    rule = rule_parse(lex);
    parser->rules[parser->rule_count] = rule;
    parser->rule_count += 1;

    // Check buffer size
    if (parser->rule_count >= parser->rule_buffer_size)
    {
        parser->rule_buffer_size += RULE_BUFFER_SIZE;
        parser->rules = realloc(parser->rules, 
            parser->rule_buffer_size * sizeof(Rule));
    }
}

void parser_parse(
    Parser *parser, 
    LexerStream *lex)
{
    Token name;

    while (!lex->eof_flag)
    {
        switch (lex->look.type)
        {
            case TinyParse_Project: parse_project_name(parser, lex); break;
            case TinyParse_Name: parse_rule(parser, lex); break;

            default: 
                lexer_error(lex, lex->look, "Unexpected token"); 
                tinyparse_next(lex); 
                break;
        }
    }
}

void parser_free(
    Parser *parser)
{
    int i;

    for (i = 0; i < parser->rule_count; i++)
        rule_free(&parser->rules[i]);

    parser->rule_count = 0;
    parser->rule_buffer_size = 0;
    free(parser->rules);
}
