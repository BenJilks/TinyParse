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

static int new_state(
    Parser *parser)
{
    parser->table_size += 1;
    parser->table = realloc(parser->table,
        parser->table_size * parser->table_width);

    memset(parser->table + 
        (parser->table_size - 1) * parser->table_width, 
        -1, parser->table_width);
    return parser->table_size - 1;
}

static int new_command(
    Parser *parser,
    int flags)
{
    Command command;

    parser->command_count += 1;
    parser->commands = realloc(parser->commands, 
        sizeof(Command) * parser->command_count);

    command.flags = flags;
    parser->commands[parser->command_count - 1] = command;

    return parser->command_count - 1;
}

static void set_transition(
    Parser *parser,
    int from_state,
    int to_state,
    int command_id,
    int token_type)
{
    int from_index;

    from_index = from_state * parser->table_width + (token_type * STATE_WIDTH);

    *(STATE_ID_TYPE*)(parser->table + from_index) = to_state;
    parser->table[from_index + STATE_ID_SIZE] = command_id;
}

static void copy_transitions(
    LexerStream *lex,
    Parser *parser,
    int copy_from_state,
    int copy_to_state,
    int to_state,
    int command_id);

static int next_consuming_state(
    Parser *parser,
    int from_state,
    int from_command_id,
    int token_type,
    int *call_stack,
    int *call_stack_pointer)
{
    int from_index;

    while (from_command_id != -1)
    {
        Command *command;

        command = &parser->commands[from_command_id];
        if (command->flags & FLAG_CALL)
        {
            int return_state;
            int return_command_id;
            LOG("Follow function call %i -> %i\n", 
                from_state, parser->rules[command->to_rule].start_index);

            from_index = from_state * parser->table_width + token_type * STATE_WIDTH;
            return_state = *(STATE_ID_TYPE*)(parser->table + from_index);
            call_stack[(*call_stack_pointer)++] = return_state;

            from_state = parser->rules[command->to_rule].start_index;
            from_command_id = parser->table[
                from_state * parser->table_width + 
                token_type * STATE_WIDTH + 
                STATE_ID_SIZE];
            continue;
        }

        if (command->flags & FLAG_MARK_TYPE ||
            command->flags & FLAG_MARK_NODE_TYPE ||
            command->flags & FLAG_NULL ||
            command->flags & FLAG_PUSH_SUB ||
            command->flags & FLAG_PEEK)
        {
            from_index = from_state * parser->table_width + token_type * STATE_WIDTH;
            LOG("Follow ignore %i -> %i\n", 
                from_state, *(STATE_ID_TYPE*)(parser->table + from_index));
            from_state = *(STATE_ID_TYPE*)(parser->table + from_index);
            from_command_id = parser->table[
                from_state * parser->table_width + token_type * STATE_WIDTH + STATE_ID_SIZE];
            continue;
        }

        if (command->flags & FLAG_RETURN)
        {
            if (*call_stack_pointer <= 0)
            {
                LOG("End of call stack\n");
                break;
            }

            from_state = call_stack[--(*call_stack_pointer)];
            from_command_id = parser->table[
                from_state * parser->table_width + token_type * STATE_WIDTH + STATE_ID_SIZE];
            LOG("Return state -> %i\n", from_state);
            continue;
        }

        if (command->flags & FLAG_SET_FLAG ||
            command->flags & FLAG_UNSET_FLAG)
        {
            return -1;
        }

        break;
    }

    return from_state;
}

static int follow_call_transision(
    Parser *parser,
    int from_state,
    int from_command_id,
    int token_type)
{
    int from_index;
    int call_stack[80];
    int call_stack_pointer;

    LOG("Trying to follow %i with token %i and command %i\n", 
        from_state, token_type, from_command_id);

    // Follow until it finds a consuming state
    call_stack_pointer = 0;
    from_state = next_consuming_state(parser, 
        from_state, from_command_id, token_type, 
        call_stack, &call_stack_pointer);

    // Follow where this state points to
    from_index = from_state * parser->table_width + token_type * STATE_WIDTH;
    LOG("Follow state %i -> %i\n", from_state, *(STATE_ID_TYPE*)(parser->table + from_index));
    from_state = *(STATE_ID_TYPE*)(parser->table + 
        from_index);
    from_command_id = parser->table[
        from_state * parser->table_width + token_type * STATE_WIDTH + STATE_ID_SIZE];

    // Find the next consuming state after then
    from_state = next_consuming_state(parser, 
        from_state, from_command_id, token_type,
        call_stack, &call_stack_pointer);
    
    return from_state;
}

static int is_return_state(
    Parser *parser,
    int state)
{
    int index;
    int first_commnad_id;
    Command *first_command;

    if (state == -1)
        return 1;

    index = state * parser->table_width;
    first_commnad_id = parser->table[index + STATE_ID_SIZE];
    if (first_commnad_id == -1)
        return 0;
    
    first_command = &parser->commands[first_commnad_id];
    return first_command->flags & FLAG_RETURN;
}

static void resolve_collition(
    LexerStream *lex,
    Parser *parser,
    int copy_from_state,
    int copy_to_state,
    int to_state_b,
    int command_id_b,
    int token_type)
{
    int next_state_a, is_return_state_a;
    int next_state_b, is_return_state_b;
    int to_state_a;
    int command_id_a;
    int peek_state;
    int peek_command_id;
    int index, i;

    debug_start_scope();
    LOG("Collition found at %i, fixing %i -> %i\n", token_type, copy_from_state, copy_to_state);

    peek_state = new_state(parser);
    peek_command_id = new_command(parser, FLAG_PEEK);

    index = copy_to_state * parser->table_width + token_type * STATE_WIDTH;
    to_state_a = *(STATE_ID_TYPE*)(parser->table + index);
    command_id_a = parser->table[index + STATE_ID_SIZE];

    next_state_a = follow_call_transision(parser, to_state_a, command_id_a, token_type);
    next_state_b = follow_call_transision(parser, to_state_b, command_id_b, token_type);
    is_return_state_a = is_return_state(parser, next_state_a);
    is_return_state_b = is_return_state(parser, next_state_b);

    if (is_return_state_a && is_return_state_b)
    {
        // TODO: handles this error
        printf("Error: Two branches are identical\n");
        return;
    }

    debug_prefix();
    for (i = 0; i < parser->table_width / STATE_WIDTH; i++)
    {
        int index_a, index_b;
        int transition_a, transition_b;

        index_a = next_state_a * parser->table_width + i * STATE_WIDTH;
        index_b = next_state_b * parser->table_width + i * STATE_WIDTH;
        transition_a = *(STATE_ID_TYPE*)(parser->table + index_a);
        transition_b = *(STATE_ID_TYPE*)(parser->table + index_b);

        // Secondary collition
        if (transition_a != -1 && transition_b != -1)
        {
            if (is_return_state_a)
            {
                _LOG("%i -> B, ", i);
                set_transition(parser, peek_state, 
                    to_state_b, command_id_b, i);
                continue;
            }
            
            if (is_return_state_b)
            {
                _LOG("%i -> A, ", i);
                set_transition(parser, peek_state, 
                    to_state_a, command_id_a, i);
                continue;
            }

            _LOG("%i -> A/B, ", i);
            printf("FIXME: handle secondary collisions\n");
            continue;
        }

        if (transition_a != -1)
        {
            _LOG("%i -> A, ", i);
            set_transition(parser, peek_state, 
                to_state_a, command_id_a, i);
            continue;
        }

        if (transition_b != -1)
        {
            _LOG("%i -> B, ", i);
            set_transition(parser, peek_state, 
                to_state_b, command_id_b, i);
            continue;
        }

        _LOG("%i -> Err, ", i);
    }
    _LOG("\n");

    set_transition(parser, copy_to_state, 
        peek_state, peek_command_id, token_type);

    debug_end_scope();
}

static void copy_transitions(
    LexerStream *lex,
    Parser *parser,
    int copy_from_state,
    int copy_to_state,
    int to_state,
    int command_id)
{
    LOG("\t=> %i -> %i (to: %i, command: %i)\n", 
        copy_from_state, copy_to_state, 
        to_state, command_id);
    int i;
    int copy_from_index;
    int copy_to_index;
    
    copy_from_index = copy_from_state * parser->table_width;
    copy_to_index = copy_to_state * parser->table_width;
    for (i = 0; i < parser->table_width / STATE_WIDTH; i++)
    {
        int from_tranition;
        int to_transition;
        int to_command_id;
        int form_command_id;

        from_tranition = *(STATE_ID_TYPE*)(parser->table + copy_from_index + i * STATE_WIDTH);
        form_command_id = parser->table[copy_from_index + i * STATE_WIDTH + STATE_ID_SIZE];
        to_transition = *(STATE_ID_TYPE*)(parser->table + copy_to_index + i * STATE_WIDTH);
        to_command_id = parser->table[copy_to_index + i * STATE_WIDTH + STATE_ID_SIZE];
        if (from_tranition != -1)
        {
            int should_overwrite;

            should_overwrite = to_command_id != -1 &&
                parser->commands[to_command_id].flags & FLAG_UNSET_FLAG;

            if (to_transition != -1 && !should_overwrite)
                resolve_collition(lex, parser, copy_from_state, copy_to_state, to_state, command_id, i);
            else
                set_transition(parser, copy_to_state, to_state, command_id, i);
        }
    }
}

static void link_sub_call(
    LexerStream *lex,
    Parser *parser,
    FSM *compiled_rules,
    FSM *from_rule,
    Link link)
{
    FSM *to_rule;
    int i, j;
    int copy_from_state; 
    int copy_to_state;
    int to_state;
    int command_id;

    to_rule = &compiled_rules[link.to_rule];
    link_rule(lex, parser, compiled_rules, to_rule);

    // Find all the transitions where the sub call 
    // links to something and add a link
    copy_from_state = to_rule->start_index;
    copy_to_state = link.from_state + from_rule->start_index;
    to_state = link.return_state + from_rule->start_index; 
    command_id = link.command_id + from_rule->command_start;
    copy_transitions(lex, parser, 
        copy_from_state, copy_to_state, 
        to_state, command_id);

    //LOG("\t=> %i -> %i\n", copy_from_state, copy_to_state);
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
