#include "FSM.h"
#include "debug.h"

#define COMMAND_BUFFER  80
#define LINK_BUFFER     80

static int new_state(
    FSM *fsm,
    Parser *parser)
{
    int last_buffer_size, buffer_size;
    int state;

    // Get next state index
    state = fsm->count;
    fsm->count += 1;

    // Check there's enough memory for the next state
    if (fsm->count >= fsm->buffer_count)
    {
        last_buffer_size = fsm->buffer_count * 
            parser->table_width;
        buffer_size = last_buffer_size + 
            BUFFER_SIZE * parser->table_width;
        fsm->buffer_count += BUFFER_SIZE;

        fsm->table = realloc(fsm->table, buffer_size);
        memset(fsm->table + last_buffer_size, -1, buffer_size - last_buffer_size);
    }

    return state;
}

static Token TK_NULL = { -1 };
static int new_command(
    FSM *fsm,
    int flags, 
    Token attr)
{
    Command command;

    // Create command struct
    command.flags = flags;
    command.node = fsm->name;
    command.attr = attr;

    // Get a new ID and add it to the FSM
    if (fsm->command_count + 1 >= fsm->command_buffer)
    {
        fsm->command_buffer += COMMAND_BUFFER;
        fsm->commands = realloc(fsm->commands, 
            sizeof(Command) * fsm->command_buffer);
    }
    fsm->commands[fsm->command_count] = command;
    fsm->command_count += 1;

    return fsm->command_count - 1;
}

static EndingStates compile_node(
    FSM *fsm,
    LexerStream *lex,
    Parser *parser,
    RuleNode *node,
    EndingStates from);

static void create_transision(
    FSM *fsm,
    Parser *parser,
    int from_state,
    int state,
    int token_index,
    int command_id)
{
    int from_index;
    int table_index;

    from_index = from_state * parser->table_width;
    table_index = from_index + token_index * STATE_WIDTH;
    fsm->table[table_index] = state;
    fsm->table[table_index + 1] = command_id;
}

static void create_all_transition(
    FSM *fsm,
    Parser *parser,
    int to_state,
    int command_id,
    EndingStates from)
{
    int i, j;

    for (i = 0; i < from.count; i++)
    {
        int from_state;

        from_state = from.states[i];
        for (j = 0; j < parser->table_width/2; j++)
        {
            create_transision(fsm, parser, 
                from_state, to_state, 
                j, command_id);
        }
    }
}

static void create_link(
    FSM *fsm,
    int to_rule,
    int return_state,
    int command_id,
    EndingStates from)
{
    Link link;

    // Create link
    link.to_rule = to_rule;
    link.return_state = return_state;
    link.command_id = command_id;
    link.from = from;
    
    // Add link to state machine
    fsm->links[fsm->link_count] = link;
    fsm->link_count += 1;
    if (fsm->link_count >= fsm->link_buffer)
    {
        fsm->link_buffer += LINK_BUFFER;
        fsm->links = realloc(fsm->links, 
            sizeof(Link) * fsm->link_buffer);
    }
}

static EndingStates compile_sub_call(
    FSM *fsm,
    LexerStream *lex,
    Parser *parser,
    Token token,
    Token label,
    int should_push,
    const char *name,
    EndingStates from)
{
    EndingStates endings;
    int rule_index;
    int return_state;
    int i, j;
    int command_id;

    return_state = new_state(fsm, parser);
    endings.count = 1;
    endings.states[0] = return_state;

    // Find the rules' index
    rule_index = parser_find_rule_index(parser, name);
    if (rule_index == -1)
    {
        lexer_error(lex, token, 
            "No rule or token with that name found");
        return endings;
    }

    // Create call command
    command_id = new_command(fsm, FLAG_CALL, TK_NULL);
    fsm->commands[command_id].to_rule = rule_index;

    // Create transitions
    create_link(fsm, rule_index, return_state, command_id, from);
    if (should_push)
    {
        int push_command_id;
        int push_state;

        push_command_id = new_command(fsm, FLAG_PUSH_SUB, label);
        push_state = new_state(fsm, parser);
        fsm->commands[push_command_id].to_rule = rule_index;
        create_all_transition(fsm, parser, push_state, push_command_id, endings);
        endings.states[0] = push_state;
    }

#if DEBUG
    for (i = 0; i < from.count; i++)
    {
        LOG("%i ==> %s ==> %i\n", from.states[i], 
            name, endings.states[0]);
    }
#endif

    return endings;
}

static EndingStates compile_match(
    FSM *fsm,
    LexerStream *lex,    
    Parser *parser,
    Token alias,
    Token label,
    int should_push,
    EndingStates from)
{
    EndingStates endings;
    int state, i;
    int token_index;
    char token_name[80];
    int command_id;

    // Find the token index from the name
    lexer_read_string(lex, alias, token_name);
    token_index = parser_find_token_id(parser, token_name);
    if (token_index == -1)
    {
        // If the token was not found, assume it's 
        // a sub rule call
        return compile_sub_call(fsm, lex, parser, 
            alias, label, should_push, token_name, from);
    }

    // Create a push command if needed
    command_id = -1;
    if (should_push)
        command_id = new_command(fsm, FLAG_SET, label);

    // Create a new state and make the 
    // transitions to it
    state = new_state(fsm, parser);
    for (i = 0; i < from.count; i++)
    {
        create_transision(fsm, parser, 
            from.states[i], state, 
            token_index, command_id);

        LOG("%i --%s--> %i\n", from.states[i], 
            token_name, state);
    }

    // Make the end this state
    endings.states[0] = state;
    endings.count = 1;
    return endings;
}

static EndingStates compile_keyword(
    FSM *fsm,
    LexerStream *lex,
    Parser *parser,
    RuleNode *node,
    EndingStates from)
{
    return compile_match(fsm, lex, parser, 
        node->value, node->label, 0, from);
}

static EndingStates compile_value(
    FSM *fsm,
    LexerStream *lex,
    Parser *parser,
    RuleNode *node,
    EndingStates from)
{
    return compile_match(fsm, lex, parser, 
        node->value, node->label, 1, from);
}

#define MAX(a, b) (a) > (b) ? (a) : (b)

static void insert_state(
	FSM *fsm,
	Parser *parser,
	int ins_state,
	int ins_command,
	EndingStates from)
{
	int i, j;
	int ins_index;

	ins_index = ins_state * parser->table_width;
	for (i = 0; i < from.count; i++)
	{
		int from_state;
		int og_index;
		
		from_state = from.states[i];
		og_index = from_state * parser->table_width;
		for (j = 0; j < parser->table_width; j+=2)
		{
			int og_transition;
			int og_command;

			og_transition = fsm->table[og_index + j];
			og_command = fsm->table[og_index + j + 1];
			if (og_transition != -1)
			{
				fsm->table[og_index + j + 0] = ins_state;
				fsm->table[og_index + j + 1] = ins_command;
				fsm->table[ins_index + j + 0] = og_transition;
				fsm->table[ins_index + j + 1] = og_command;
			}
		}
	}
}

static void mark_type(
    FSM *fsm,
    Parser *parser,
    Token label,
    EndingStates from)
{
    int command_id;
    int type_state;

    // Create command and state
    command_id = new_command(fsm, 
        FLAG_MARK_TYPE, label);
    type_state = new_state(fsm, parser);

    // Create transitions
	insert_state(fsm, parser, 
		type_state, command_id, from);
}

static EndingStates compile_or(
    FSM *fsm,
    LexerStream *lex,
    Parser *parser,
    RuleNode *node,
    EndingStates from)
{
    EndingStates endings, branch;
    RuleNode *curr;

    LOG("If =>\n");
    debug_start_scope();

    // Set up ending data
    endings.count = 0;

    curr = node->child;
    while (curr != NULL)
    {
        int i;

        // Compile a branch of the if statement from the 
        // same point and record the endings
        branch = compile_node(fsm, lex, parser, curr, from);
        for (i = 0; i < branch.count; i++)
        {
            endings.states[endings.count] = branch.states[i];
            endings.count += 1;
        }

        // Increment total size and move to the next branch
        curr = curr->next;
    }

    // If there's a type label, create 
    // a command node to mark the type
    if (node->has_label)
        mark_type(fsm, parser, node->label, from);

    debug_end_scope();
    return endings;
}

static EndingStates compile_optional(
    FSM *fsm,
    LexerStream *lex,
    Parser *parser,
    RuleNode *node,
    EndingStates from)
{
    EndingStates endings;
    int null_state;
    int null_command;

    LOG("Optional =>\n");
    debug_start_scope();

    // If there's a label, then initialize its value
    if (node->has_label)
    {
        int state, command;

        state = new_state(fsm, parser);
        command = new_command(fsm, FLAG_SET_FLAG, node->label);
        create_all_transition(fsm, parser, state, command, from);

        null_command = new_command(fsm, FLAG_UNSET_FLAG, node->label);
        from.states[0] = state;
        from.count = 1;
    }
    else
    {
        null_command = new_command(fsm, FLAG_NULL, TK_NULL);
    }

    // Create default null transitions
    null_state = new_state(fsm, parser);
    create_all_transition(fsm, parser, null_state, null_command, from);

#if DEBUG
    int i;
    for (i = 0; i < from.count; i++)
        LOG("%i --*--> %i\n", from.states[i], null_state);
#endif

    // Overwrite transition for valid ones
    endings = compile_node(fsm, lex, 
        parser, node->child, from);
    debug_end_scope();

    endings.states[endings.count] = null_state;
    endings.count += 1;
    return endings;
}

static EndingStates compile_expression(
    FSM *fsm,
    LexerStream *lex,
    Parser *parser,
    RuleNode *node,
    EndingStates from)
{
    EndingStates endings;

    endings = compile_node(fsm, lex, parser, node, from);
    if (node->next != NULL)
    {
        // If there's another node in the 
        // sequence, compile that
        return compile_expression(fsm, lex, parser, 
            node->next, endings);
    }
    return endings;
}

static EndingStates compile_node(
    FSM *fsm,
    LexerStream *lex,
    Parser *parser,
    RuleNode *node,
    EndingStates from)
{
    EndingStates endings;
    switch(node->type)
    {
        case RULE_KEYWORD: endings = compile_keyword(fsm, lex, parser, node, from); break;
        case RULE_VALUE: endings = compile_value(fsm, lex, parser, node, from); break;
        case RULE_OR: endings = compile_or(fsm, lex, parser, node, from); break;
        case RULE_OPTIONAL: endings = compile_optional(fsm, lex, parser, node, from); break;
        case RULE_EXPRESSION: 
            LOG("Expression => \n");
            debug_start_scope();
            endings = compile_expression(fsm, lex, parser, node->child, from);
            debug_end_scope();
            break;
    }

    return endings;
}

static EndingStates create_ending_transitions(
    FSM *fsm,
    Parser *parser,
    EndingStates from)
{
    EndingStates endings;
    int ending_state;
    int i, j;
    int command_id;

    // Create transisitions to ending state
    ending_state = new_state(fsm, parser);
    command_id = new_command(fsm, FLAG_RETURN, TK_NULL);
    for (i = 0; i < from.count; i++)
    {
        int state_index, token_index;
        LOG("\t%i --*--> end\n", from.states[i]);

        state_index = from.states[i] * parser->table_width;
        for (j = 0; j < parser->table_width; j += STATE_WIDTH)
        {
            token_index = state_index + j;
            fsm->table[token_index + 0] = ending_state;
            fsm->table[token_index + 1] = command_id;
        }
    }

    // Create and return ending state
    endings.count = 1;
    endings.states[0] = ending_state;
    return endings;
}

FSM fsm_compile(
    Rule *rule,
    LexerStream *lex,
    Parser *parser)
{
    EndingStates from, endings;
    FSM fsm;
    int start;
    LOG("Compiling rule '%s'\n", rule->name);

    // Create the new FSM
    fsm.name = rule->name_token;
    fsm.count = 0;
    fsm.buffer_count = 0;
    fsm.table = malloc(1);

    // Allocate command table
    fsm.command_buffer = COMMAND_BUFFER;
    fsm.command_count = 0;
    fsm.commands = malloc(sizeof(Command) 
        * fsm.command_buffer);

    // Allocate link table
    fsm.link_buffer = 80;
    fsm.links = malloc(sizeof(Link) * fsm.link_buffer);
    fsm.link_count = 0;
    fsm.has_been_linked = 0;

    // Create a starting state
    start = new_state(&fsm, parser);
    from.states[0] = start;
    from.count = 1;

    // Compile the rule
    endings = compile_expression(&fsm, lex, parser, 
        rule->root, from);
    
    // Create return transitions 
    // to the ending state
    fsm.endings = create_ending_transitions(
        &fsm, parser, endings);

    return fsm;
}

void fsm_free(
    FSM *fsm)
{
    free(fsm->table);
    free(fsm->commands);
    free(fsm->links);
}
