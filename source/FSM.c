#include "FSM.h"
#include "debug.h"

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

static EndingStates compile_node(
    FSM *fsm,
    LexerStream *lex,
    Parser *parser,
    RuleNode *node,
    EndingStates from);

static void calculate_branch_size(
    EndingStates *endings,
    EndingStates *from,
    int commands,
    int element_size)
{
    // This branches size if the last ones' total 
    // size plus this element size if it's stored
    endings->branch_size[0] = from->total_size;
    if (commands & COMMAND_PUSH)
        endings->branch_size[0] += element_size;

    // As there's only one branch, the total size 
    // is the branch size
    endings->total_size = endings->branch_size[0];
}

static EndingStates compile_sub_call(
    FSM *fsm,
    LexerStream *lex,    
    Parser *parser,
    Token token,
    const char *name,
    int commands,
    EndingStates from)
{
    EndingStates endings;
    int rule_index;
    int return_state;

    // Find the rules' index
    rule_index = parser_find_rule_index(parser, name);
    if (rule_index == -1)
    {
        lexer_error(lex, token, 
            "No rule or token with that name found");
        return endings;
    }

    // Create return state
    calculate_branch_size(&endings, &from, 
        commands, sizeof(void*));
    return_state = new_state(fsm, parser);
    endings.states[0] = return_state;
    endings.count = 1;

    // Create linking data to be linked later
    Link link;
    link.from_states = from;
    link.to_state = return_state;
    link.to_rule = rule_index;
    link.commands = commands | COMMAND_CALL;
    if (link.commands & COMMAND_PUSH)
    {
        link.commands ^= COMMAND_PUSH;
        link.commands |= COMMAND_PUSH_SUB;
    }
    fsm->links[fsm->link_count] = link;
    fsm->link_count += 1;

    return endings;
}

static EndingStates compile_match(
    FSM *fsm,
    LexerStream *lex,    
    Parser *parser,
    Token alias,
    int commands,
    EndingStates from)
{
    EndingStates endings;
    int state, i;
    int token_index;
    char token_name[80];

    // Find the token index from the name
    lexer_read_string(lex, alias, token_name);
    token_index = parser_find_token_id(parser, token_name);
    if (token_index == -1)
    {
        // If the token was not found, assume it's 
        // a sub rule call
        return compile_sub_call(fsm, lex, parser, 
            alias, token_name, commands, from);
    }

    // Create a new state and make the 
    // transitions to it
    state = new_state(fsm, parser);
    for (i = 0; i < from.count; i++)
    {
        int from_index;
        int table_index;

        from_index = from.states[i] * parser->table_width;
        table_index = from_index + token_index * STATE_WIDTH;
        fsm->table[table_index] = state;
        fsm->table[table_index + 1] = commands;
        LOG("%i --%s--> %i\n", from.states[i], 
            token_name, state);
    }

    // Make the end this state
    calculate_branch_size(&endings, &from, 
        commands, sizeof(Token));
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
        node->value, COMMAND_NOP, from);
}

static EndingStates compile_value(
    FSM *fsm,
    LexerStream *lex,
    Parser *parser,
    RuleNode *node,
    EndingStates from)
{
    return compile_match(fsm, lex, parser, 
        node->value, COMMAND_PUSH, from);
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
            endings.branch_size[endings.count] = branch.branch_size[i];
            endings.count += 1;
        }

        // Increment total size and move to the next branch
        endings.total_size += branch.total_size;
        curr = curr->next;
    }

    debug_end_scope();
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
        case RULE_EXPRESSION: 
            LOG("Expression => \n");
            debug_start_scope();
            endings = compile_expression(fsm, lex, parser, node->child, from); break;
            debug_end_scope();
    }

    return endings;
}

FSM fsm_compile(
    Rule *rule,
    LexerStream *lex,
    Parser *parser)
{
    EndingStates from;
    FSM fsm;
    int start;
    LOG("Compiling rule '%s'\n", rule->name);

    // Create the new FSM
    fsm.count = 0;
    fsm.buffer_count = 0;
    fsm.link_count = 0;
    fsm.table = malloc(1);

    // Create a starting state
    start = new_state(&fsm, parser);
    from.states[0] = start;
    from.count = 1;

    // Compile the rule
    fsm.endings = compile_expression(&fsm, lex, parser, 
        rule->root, from);

    return fsm;
}
