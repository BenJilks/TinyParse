#include "parser.h"
#include "debug.h"
#include "FSM.h"

#define RULE_BUFFER_SIZE 80

Parser parser_create()
{
    Parser parser;
    parser.rules = malloc(RULE_BUFFER_SIZE * sizeof(Rule));
    parser.rule_buffer_size = RULE_BUFFER_SIZE;
    parser.rule_count = 0;

    // Create token buffer
    parser.token_buffer = TOKEN_BUFFER;
    parser.tokens = (char*)malloc(
        parser.token_buffer * TOKEN_LEN * 2);
    parser.token_count = 0;

    return parser;
}

Rule *parser_find_rule(
    Parser *parser, 
    const char *name)
{
    int index;
    index = parser_find_rule_index(parser, name);

    return index != -1 ? 
        &parser->rules[index] :
        NULL;
}

int parser_find_rule_index(
    Parser *parser, 
    const char *name)
{
    int i;

    for (i = 0; i < parser->rule_count; i++)
    {
        if (!strcmp(parser->rules[i].name, name))
            return i;
    }
    
    return -1;
}

int parser_find_token_id(
    Parser *parser, 
    const char *alias)
{
    char *other_alias;
    int i;

    for (i = 0; i < parser->token_count; i++)
    {
        other_alias = parser->tokens + i * TOKEN_LEN * 2;
        if (!strcmp(other_alias, alias))
            return i;
    }

    return -1;
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

static void parse_define_tokens(
    Parser *parser,
    LexerStream *lex)
{
    Token alias_token;
    Token name_token;
    int token_index;

    LOG("Defining tokens\n");

    // Define all types in the format 
    // ( <Alias> <Name> ... )
    tinyparse_match(lex, TinyParse_Open, "(");
    while (lex->look.type != TinyParse_Close)
    {
        // Parse token data
        alias_token = tinyparse_match(lex, TinyParse_Name, "Alias");
        name_token = tinyparse_match(lex, TinyParse_Name, "Name");

        // Read data into the token buffer
        token_index = parser->token_count * TOKEN_LEN * 2;
        lexer_read_string(lex, alias_token, parser->tokens + token_index);
        lexer_read_string(lex, name_token, parser->tokens + token_index + TOKEN_LEN);
        parser->token_count += 1;

        // Debug statement
        LOG("\t => %s -> '%s'\n", 
            parser->tokens + token_index + TOKEN_LEN, 
            parser->tokens + token_index);

        // Check to see if there's any memory left for the next token,
        // otherwise allocate more
        if (parser->token_count >= parser->token_buffer)
        {
            parser->token_buffer += TOKEN_BUFFER;
            parser->tokens = realloc(parser->tokens, 
                parser->token_buffer * TOKEN_LEN * 2);
        }
    }
    tinyparse_match(lex, TinyParse_Close, ")");
}

static void parse_define(
    Parser *parser,
    LexerStream *lex)
{
    Token type_token;
    char type_name[80];

    // Find the type to define
    tinyparse_match(lex, TinyParse_Define, "define");
    type_token = tinyparse_match(lex, TinyParse_Name, "Type");
    lexer_read_string(lex, type_token, type_name);

    // Decode this type name
    if (!strcmp(type_name, "tokens"))
        parse_define_tokens(parser, lex);
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
            case TinyParse_Define: parse_define(parser, lex); break;
            case TinyParse_Name: parse_rule(parser, lex); break;

            default: 
                lexer_error(lex, lex->look, "Unexpected token");
                tinyparse_next(lex);
                break;
        }
    }
}

static void link_state(
    Parser *parser,
    FSM *rule,
    int from_state,
    int to_state,
    int commands)
{
    int start, i;
    int index;
    char transition;
    
    // Copy the transitions from the to state into the from state,
    // this will make it so it jumps into the sub state
    start = rule->start_index;
    index = from_state * parser->table_width;
    for (i = 0; i < parser->table_width; i += 2)
    {
        transition = parser->table[to_state * parser->table_width + i];
        if (transition != -1)
        {
            parser->table[index + i] = transition;
            parser->table[index + i + 1] = commands;
        }
    }
}

static void link_rule(
    Parser *parser, 
    FSM *rule,
    FSM *compiled_rules)
{
    int i, j;
    int to_state, from_state;
    Link link;

    for (i = 0; i < rule->link_count; i++)
    {
        link = rule->links[i];
        to_state = compiled_rules[link.to_rule].start_index;

        // For each state in each link
        for (j = 0; j < link.from_states.count; j++)
        {
            from_state = link.from_states.states[i]; 
            LOG("\t => Link %i -> %i\n", from_state, to_state);

            link_state(parser, rule, from_state, 
                to_state, link.commands);
        }
    }
}

static void link_compiled_rules(
    Parser *parser, 
    FSM *compiled_rules)
{
    FSM *rule;
    int start, to_state;
    int i, j;

    // Find total table size
    parser->table_size = 0;
    for (i = 0; i < parser->rule_count; i++)
    {
        rule = &compiled_rules[i];

        rule->start_index = parser->table_size;
        parser->table_size += rule->count;
    }

    // Allocate table
    parser->table = malloc(parser->table_size * 
        parser->table_width);

    // Link tables
    for (i = 0; i < parser->rule_count; i++)
    {
        // Copy table rows into the main table, fixing state indices
        rule = &compiled_rules[i];
        start = rule->start_index * parser->table_width;
        for (j = 0; j < rule->count * parser->table_width; j += 2)
        {
            to_state = rule->table[j];
            if (to_state != -1)
                to_state += rule->start_index;

            parser->table[start + j] = to_state;
            parser->table[start + j + 1] = rule->table[j + 1];
        }
    }

    // Parse links to sub rules
    for (i = 0; i < parser->rule_count; i++)
        link_rule(parser, &compiled_rules[i], compiled_rules);
}

void parser_compile_and_link(
    Parser *parser, 
    LexerStream *lex)
{
    FSM *compiled_rules;
    int i;

    // Calculate the width of each 
    // row in the parser table
    parser->table_width = parser->token_count * 2;

    // Compile each rule
    compiled_rules = malloc(parser->rule_count * sizeof(FSM));
    for (i = 0; i < parser->rule_count; i++)
    {
        compiled_rules[i] = fsm_compile(
            &parser->rules[i], lex, parser);
    }

    // Link and free
    link_compiled_rules(parser, compiled_rules);
    free(compiled_rules);
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
