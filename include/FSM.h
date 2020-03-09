#ifndef FSM_H
#define FSM_H

#include "rule.h"
#include "parser.h"

#define BUFFER_SIZE 80
#define STATE_WIDTH 2

typedef struct _EndingStates
{
    int states[80];
    int count;
} EndingStates;

typedef struct _Link
{
    EndingStates from;
    int to_rule;
    int return_state;
    int command_id;
} Link;

typedef struct _FSM
{
    // Table data
    Token name;
    char *table;
    int count, buffer_count;
    EndingStates endings;

    // Commands
    Command *commands;
    int command_count;
    int command_buffer;
    int command_start;

    // Linking data
    Link *links;
    int link_count;
    int link_buffer;
    int has_been_linked;
    int start_index;
} FSM;

FSM fsm_compile(Rule *rule, LexerStream *lex, Parser *parser);
void fsm_free(FSM *fsm);

#endif // FSM_H
