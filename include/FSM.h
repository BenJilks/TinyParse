#ifndef FSM_H
#define FSM_H

#include "rule.h"
#include "parser.h"

#define BUFFER_SIZE 80
#define STATE_WIDTH 3

#define COMMAND_NOP         0b000000
#define COMMAND_PUSH        0b100000
#define COMMAND_CALL        0b010000
#define COMMAND_RETURN      0b001000
#define COMMAND_PUSH_SUB    0b000100
#define COMMAND_PADDING     0b000010
#define COMMAND_MARK_TYPE   0b000001

typedef struct _EndingStates
{
    int states[80];
    int count;

    // Flag for saving the token 
    // type to the current node
    int mark_type;

    // Size information
    int branch_size[80];
    int total_size;
} EndingStates;

typedef struct _Link
{
    EndingStates from_states;
    int to_state, to_rule;
    int commands;
} Link;

typedef struct _FSM
{
    // Table data
    char *table;
    int count, buffer_count;
    EndingStates endings;

    // Linkings to sub rule calls
    Link links[80];
    int link_count;
    int start_index;
} FSM;

FSM fsm_compile(Rule *rule, LexerStream *lex, Parser *parser);

#endif // FSM_H
