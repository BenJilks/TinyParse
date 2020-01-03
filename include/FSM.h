#ifndef FSM_H
#define FSM_H

#include "rule.h"
#include "parser.h"

typedef struct _EndingStates
{
    int count;
    int states[80];
} EndingStates;

typedef struct _Link
{
    EndingStates from_states;
    int to_rule;
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
