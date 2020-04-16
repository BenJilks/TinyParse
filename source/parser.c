#include "parser.h"
#include "debug.h"
#include "FSM.h"
#include "tinylex_intergration.h"

#define RULE_BUFFER_SIZE 80

Parser parser_create()
{
    Parser parser;
    parser.rules = malloc(RULE_BUFFER_SIZE * sizeof(Rule));
    parser.rule_buffer_size = RULE_BUFFER_SIZE;
    parser.rule_count = 0;
    parser.commands = malloc(1);
    parser.command_count = 0;

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

void parser_add_token(
	Parser *parser,
	const char *name,
	const char *value)
{
	int token_index;

	// Copy token to the parser
    token_index = parser->token_count * TOKEN_LEN * 2;
	strcpy(parser->tokens + token_index, name);
	strcpy(parser->tokens + token_index + TOKEN_LEN, value);
	parser->token_count += 1;

	// Check to see if there's any memory left for the next token,
	// otherwise allocate more
	if (parser->token_count >= parser->token_buffer)
	{
		parser->token_buffer += TOKEN_BUFFER;
		parser->tokens = realloc(parser->tokens, 
			parser->token_buffer * TOKEN_LEN * 2);
	}

	// Debug statement
	LOG("\t => %s -> '%s'\n", 
		parser->tokens + token_index + TOKEN_LEN, 
		parser->tokens + token_index);
}

static void parse_define_tokens(
    Parser *parser,
    LexerStream *lex)
{
    Token alias_token;
    Token name_token;

    LOG("Defining tokens\n");

    // Define all types in the format 
    // ( <Alias> <Name> ... )
    tinyparse_match(lex, TinyParse_Open, "(");
    while (lex->look.type != TinyParse_Close)
    {
		char name[80], value[80];

        // Parse token data
        alias_token = tinyparse_next(lex);
        name_token = tinyparse_match(lex, TinyParse_Name, "Name");

        // Read data into the token buffer
        lexer_read_string(lex, alias_token, name);
        lexer_read_string(lex, name_token, value);
		parser_add_token(parser, name, value);
    }
    tinyparse_match(lex, TinyParse_Close, ")");

    parser->token_count += 1;
}

static void parse_tinylexer(
	Parser *parser,
	LexerStream *lex)
{
	Token file_path_token;
	char file_path[80];

	file_path_token = tinyparse_next(lex);
	lexer_read_string(lex, file_path_token, file_path);
	LOG("Lexer: %s\n", file_path);

	parse_tinylex(parser, file_path);
    parser->token_count += 1;
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
	else if (!strcmp(type_name, "tinylexer"))
		parse_tinylexer(parser, lex);
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

static void link_rule(
    LexerStream *lex,
    Parser *parser,
    FSM *compiled_rules,
    FSM *rule);

static void link_sub_call(
    LexerStream *lex,
    Parser *parser,
    FSM *compiled_rules,
    FSM *from_rule,
    Link link)
{
    FSM *to_rule;
    int i, j;
    int from, to;
    int to_index, from_index;

    to_rule = &compiled_rules[link.to_rule];
    to = to_rule->start_index;
    link_rule(lex, parser, compiled_rules, to_rule);

    // Find all the transitions where the sub call 
    // links to something and add a link
    from = link.from_state + from_rule->start_index;
    from_index = from * parser->table_width;
    to_index = to * parser->table_width;
    for (j = 0; j < parser->table_width; j += STATE_WIDTH)
    {
        int transition;

        transition = parser->table[to_index + j];
        if (transition != -1)
        {
            *(STATE_ID_TYPE*)(parser->table + from_index + j) = 
                link.return_state + from_rule->start_index;
            parser->table[from_index + j + STATE_ID_SIZE] = 
                link.command_id + from_rule->command_start;
        }
    }

    LOG("\t=> %i -> %i\n", from, to);
}

static void link_rule(
    LexerStream *lex,
    Parser *parser,
    FSM *compiled_rules,
    FSM *rule)
{
    int i;

    // Make sure we don't link a rule 
    // more then once
    if (rule->has_been_linked)
        return;
    rule->has_been_linked = 1;

    char name[80];
    lexer_read_string(lex, rule->name, name);
    LOG("Linking rule %s\n", name);

    for (i = 0; i < rule->link_count; i++)
    {
        link_sub_call(lex, parser, compiled_rules, 
            rule, rule->links[i]);
    }
}

static int link_compiled_rules(
    LexerStream *lex,
    Parser *parser, 
    FSM *compiled_rules,
    const char *entry_point)
{
    FSM *rule;
    int start;
    int entry_index;
    int i, j;
    int table_size;

    // Find total table size
    parser->table_size = 0;
    parser->entry_index = -1;
    for (i = 0; i < parser->rule_count; i++)
    {
        rule = &compiled_rules[i];
        if (!strcmp(parser->rules[i].name, entry_point))
            parser->entry_index = parser->table_size;

        // Mark rule positions
        rule->command_start = parser->command_count;
        rule->start_index = parser->table_size;
        parser->rules[i].start_index = parser->table_size;
        parser->table_size += rule->count;

        // Add commands to parser
        parser->command_count += rule->command_count;
        parser->commands = realloc(parser->commands, 
            sizeof(Command) * parser->command_count);
        memcpy(parser->commands + parser->command_count - rule->command_count,
            rule->commands, sizeof(Command) * rule->command_count);
    }

    // If no entry point was found
    if (parser->entry_index == -1)
    {
        printf("Error: Could not find entry point '%s'\n", 
            entry_point);
        return -1;
    }

    // Allocate table
    table_size = parser->table_size * 
        parser->table_width;
    parser->table = malloc(table_size);
    memset(parser->table, -1, table_size);

    // Link tables
    for (i = 0; i < parser->rule_count; i++)
    {
        // Copy table rows into the main table, fixing state indices
        rule = &compiled_rules[i];
        start = rule->start_index * parser->table_width;
        for (j = 0; j < rule->count * parser->table_width; j += STATE_WIDTH)
        {
            int to_state, command;

            // Find new state values
            to_state = *(STATE_ID_TYPE*)(rule->table + j) != -1 ? 
                (*(STATE_ID_TYPE*)(rule->table + j) + rule->start_index) : -1;
            command = rule->table[j + STATE_ID_SIZE] != -1 ?
                rule->table[j + STATE_ID_SIZE] + rule->command_start : -1;

            // Add them to the table
            *(STATE_ID_TYPE*)(parser->table + start + j) = to_state;
            parser->table[start + j + STATE_ID_SIZE] = command;
        }
    }

    // Link sub calls
    for (i = 0; i < parser->rule_count; i++)
        link_rule(lex, parser, compiled_rules, &compiled_rules[i]);
    
    return 0;
}

int parser_compile_and_link(
    Parser *parser, 
    LexerStream *lex,
    const char *entry_point)
{
    FSM *compiled_rules;
    int i, success;

    // Calculate the width of each 
    // row in the parser table
    parser->table_width = parser->token_count * STATE_WIDTH;

    // Compile each rule
    compiled_rules = malloc(parser->rule_count * sizeof(FSM));
    for (i = 0; i < parser->rule_count; i++)
    {
        compiled_rules[i] = fsm_compile(
            &parser->rules[i], lex, parser);
    }

    // Link and free
    success = link_compiled_rules(lex, parser, 
        compiled_rules, entry_point);
    
    free(compiled_rules);
    return success;
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
