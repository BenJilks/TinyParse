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

    // Create linking data to be linked later
    Link link;
    link.from_states = from;
    link.to_rule = rule_index;
    link.commands = commands | COMMAND_CALL;
    fsm->links[fsm->link_count] = link;
    fsm->link_count += 1;

    // Create return state
    return_state = new_state(fsm, parser);
    endings.states[0] = return_state;
    endings.count = 1;

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
    printf("%s -> %i\n", token_name, token_index);

    // Create a new state and make the 
    // transitions to it
    state = new_state(fsm, parser);
    for (i = 0; i < from.count; i++)
    {
        int from_index;

        from_index = from.states[i] * parser->table_width;
        fsm->table[from_index + token_index * 2] = state;
        fsm->table[from_index + token_index * 2 + 1] = commands;
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

static EndingStates compile_node(
    FSM *fsm,
    LexerStream *lex,
    Parser *parser,
    RuleNode *node,
    EndingStates from)
{
    EndingStates endings;

    // If the node is null, there's 
    // nothing to compile
    if (node == NULL)
        return from;

    switch(node->type)
    {
        case RULE_KEYWORD: endings = compile_keyword(fsm, lex, parser, node, from); break;
        case RULE_VALUE: endings = compile_value(fsm, lex, parser, node, from); break;
        case RULE_EXPRESSION: endings = compile_node(fsm, lex, parser, node->child, from); break;
    }

    endings = compile_node(fsm, lex, parser, node->next, endings);
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
    fsm.count = 0;
    fsm.buffer_count = 0;
    fsm.link_count = 0;
    fsm.table = malloc(1);

    // Create a starting state
    start = new_state(&fsm, parser);
    from.states[0] = start;
    from.count = 1;

    // Compile the rule
    endings = compile_node(&fsm, lex, parser, 
        rule->root, from);

    return fsm;
}
